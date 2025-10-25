#include "debouncer.h"
#include "efsw/efsw.hpp"
#include <cassert>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

namespace app::debouncer {

bool DebouncedEvent::is_shutdown() const noexcept {
  return kind == DebouncedEventKind::Shutdown;
}

bool DebouncedEvent::is_empty_payload() const noexcept {
  return kind != DebouncedEventKind::Shutdown && path.empty();
}

EventData EventData::now() {
  auto const instance = Clock::now();
  return EventData{instance, instance};
}

void EventData::touch() { update = Clock::now(); }

Clock::duration EventData::since_insert() const {
  return Clock::now() - insert;
}

Clock::duration EventData::since_update() const {
  return Clock::now() - update;
}

void Sender::send(const DebouncedEvent &e) { q->enqueue(e); }

void Receiver::recv(DebouncedEvent &e) { q->wait_dequeue(e); }

void Receiver::recv_timed(DebouncedEvent &out,
                          std::chrono::milliseconds timeout) {
  q->wait_dequeue_timed(out, timeout);
}

AsyncWatcherDebouncer::AsyncWatcherDebouncer(milliseconds delay, Sender out)
    : delay_(delay), out_(std::move(out)) {
  stop_.store(false, std::memory_order_relaxed);
  worker_ = std::thread([this] { loop(); });
}

AsyncWatcherDebouncer::~AsyncWatcherDebouncer() { stop(); }

void AsyncWatcherDebouncer::push_raw(const DebouncedEvent &e) {
  in_.enqueue(e);
}

void AsyncWatcherDebouncer::stop() {
  bool expected = false;

  if (stop_.compare_exchange_strong(expected, true,
                                    std::memory_order_relaxed)) {
    std::cout << "stop" << std::endl;
    in_.enqueue({"/tmp/c.txt", DebouncedEventKind::Shutdown});
    if (worker_.joinable())
      worker_.join();
  }
}

void AsyncWatcherDebouncer::loop() {
  DebouncedEvent ev;

  while (!stop_.load(std::memory_order_relaxed)) {
    if (!in_.wait_dequeue_timed(ev, delay_)) {
      continue;
    }

    if (ev.is_shutdown())
      break;

    std::unordered_map<std::string, DebouncedEvent> bucket;
    bucket.emplace(ev.path, ev);

    const auto deadline = Clock::now() + delay_;

    for (;;) {
      const auto now = Clock::now();
      if (now >= deadline) {
        break;
      }

      const auto left =
          std::chrono::duration_cast<milliseconds>(deadline - now);

      DebouncedEvent tmp;
      if (!in_.wait_dequeue_timed(tmp, left))
        break;

      if (tmp.is_shutdown()) {
        publish(bucket);
        return;
      }

      bucket.insert_or_assign(tmp.path, std::move(tmp));
    }

    for (const auto &[key, ev] : bucket) {
      std::cout << key << " " << ev.path.string() << static_cast<int>(ev.kind)
                << std::endl;
    }
  }
}

void AsyncWatcherDebouncer::publish(
    std::unordered_map<std::string, DebouncedEvent> &bucket) {
  for (const auto &kv : bucket) {
    kv.second.kind == DebouncedEventKind::Shutdown ? void()
                                                   : out_.send(kv.second);
  }
}

std::pair<std::unique_ptr<AsyncWatcherDebouncer>, Receiver>
init_debouncer(std::chrono::milliseconds delay) {
  auto concurrent_queue = std::make_shared<Queue>();
  Sender tx{concurrent_queue};
  Receiver rx{concurrent_queue};

  auto deb = std::make_unique<AsyncWatcherDebouncer>(delay, tx);

  return {std::move(deb), rx};
}


constexpr DebouncedEventKind from_efsw(FileAction action) noexcept {
    using A = efsw::Action;

    switch (action) {
        case A::Add: return DebouncedEventKind::Insert;
        case A::Delete: return DebouncedEventKind::Remove;
        case A::Modified: return DebouncedEventKind::Update;
        // TODO: update
        case A::Moved: return DebouncedEventKind::Update;
    };
}


FileDebouncer::FileDebouncer() {
    auto [debouncer, rx] = init_debouncer(std::chrono::milliseconds(1500));
    debouncer_ = std::move(debouncer);
    rx_ = std::move(rx);
}


void FileDebouncer::join_producers() {
  std::cout << threads_.size() << std::endl; 

  for (auto& t: threads_) {
    if (t.joinable()) { t.join(); }
  };
}

void FileDebouncer::listen(DebouncerCallback callback) {
    using namespace app::debouncer;

      std::thread consumer([&] {
      try {
        // endless loop, watch
        // should we rewrite here? idk
        for (;;) {
          join_producers();

          DebouncedEvent e;
          rx_.recv(e);

          if (e.is_shutdown())
            break;

          if (e.is_empty_payload()) {
            throw std::runtime_error("debounced event has empty path");
          }

          callback(e);
        }
      } catch (const std::exception &ex) {
        throw std::runtime_error("debouncer error (in consumer)");
      }
    });

    // producer.join();
    // deb->stop();
    consumer.join();
};

void FileDebouncer::add_thread(
    fs::path fullpath,
    FileAction action
) {
    DebouncedEventKind kind = from_efsw(action);

    std::cout << fullpath << std::endl;
    std::cout << action << std::endl;

    std::thread producer([&] {
      debouncer_->push_raw({fullpath, kind});
    });

    threads_.emplace_back(std::move(producer));
};
 

} // namespace app::debouncer
