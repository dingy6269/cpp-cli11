#pragma once
#include "blockingconcurrentqueue.h"
#include "efsw/efsw.hpp"
#include <atomic>
#include <chrono>
#include <deque>
#include <filesystem>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>

namespace app::debouncer {
using Clock = std::chrono::steady_clock;
using milliseconds = std::chrono::milliseconds;

enum class DebouncedEventKind { Update, Insert, Remove, Shutdown };

constexpr DebouncedEventKind from_efsw(efsw::Action action) noexcept;

struct DebouncedEvent {
  std::filesystem::path path;
  DebouncedEventKind kind;

  [[nodiscard]] bool is_shutdown() const noexcept;
  [[nodiscard]] bool is_empty_payload() const noexcept;
};

using Queue = moodycamel::BlockingConcurrentQueue<DebouncedEvent>;

struct EventData {
  Clock::time_point insert;
  Clock::time_point update;

  static EventData now();
  void touch();
  [[nodiscard]] Clock::duration since_insert() const;
  [[nodiscard]] Clock::duration since_update() const;
};

struct Sender {
  std::shared_ptr<Queue> q;
  void send(const DebouncedEvent &e);
};

struct Receiver {
  std::shared_ptr<Queue> q;
  void recv(DebouncedEvent &e);
  void recv_timed(DebouncedEvent &out, std::chrono::milliseconds timeout);
};

class AsyncWatcherDebouncer {
public:
  AsyncWatcherDebouncer(milliseconds delay, Sender out);
  ~AsyncWatcherDebouncer();

  void push_raw(const DebouncedEvent &e);
  void stop();

private:
  void loop();
  void publish_event(DebouncedEvent event);

  moodycamel::BlockingConcurrentQueue<DebouncedEvent> in_;
  milliseconds delay_;
  std::thread worker_;
  Sender out_;
  std::atomic<bool> stop_;
};

using FileAction = efsw::Action;
using DebouncerCallback = std::function<void(DebouncedEvent)>;

class FileDebouncer {
public:
  explicit FileDebouncer();
  ~FileDebouncer() = default;

  void listen(DebouncerCallback callback);

  // TODO: add oldFilename
  void add_thread(std::filesystem::path fullpath, efsw::Action action);

private:
  void join_producers();

  std::deque<std::thread> threads_;
  std::unique_ptr<AsyncWatcherDebouncer> debouncer_;
  Receiver rx_;
};

std::pair<std::unique_ptr<AsyncWatcherDebouncer>, Receiver>
init_debouncer(std::chrono::milliseconds delay = milliseconds(1500));

} // namespace app::debouncer