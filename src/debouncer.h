#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>
#include <filesystem>

#include "blockingconcurrentqueue.h" // из ca

// temporary
#include <iostream>

using Clock = std::chrono::steady_clock;

using milliseconds = std::chrono::milliseconds;

enum class DebouncedEventKind
{
    Update,
    Insert,
    Remove,
    Shutdown
};
//
struct DebouncedEvent
{
    std::filesystem::path path;
    DebouncedEventKind kind;

    [[nodiscard]] bool is_shutdown() const noexcept {
        return kind == DebouncedEventKind::Shutdown;
    }

    [[nodiscard]] bool is_empty_payload() const noexcept {
        return kind != DebouncedEventKind::Shutdown && path.empty();
    }
};

using Queue = moodycamel::BlockingConcurrentQueue<DebouncedEvent>;

// TODO: unused
struct EventData
{
    Clock::time_point insert;
    Clock::time_point update;

    static EventData now()
    {
        auto const instance = Clock::now();

        return EventData{instance, instance};
    }

    void touch()
    {
        update = Clock::now();
    };

    [[nodiscard]] Clock::duration since_insert() const { return Clock::now() - insert; };

    [[nodiscard]] Clock::duration since_update() const { return Clock::now() - update; }
};

///// CORE LOGIC
//
//
struct Sender
{
    std::shared_ptr<Queue> q;
    void send(const DebouncedEvent &e)
    {
        q->enqueue(e);
    }
};

struct Receiver
{
    std::shared_ptr<Queue> q;
    void recv(DebouncedEvent &e)
    {
        q->wait_dequeue(e);
    }
    void recv_timed(DebouncedEvent &out, std::chrono::milliseconds timeout)
    {
        q->wait_dequeue_timed(out, timeout);
    }
};

class AsyncWatcherDebouncer
{
public:
    AsyncWatcherDebouncer(milliseconds delay, Sender out) : delay_(delay), out_(std::move(out))
    {
        stop_.store(false, std::memory_order_relaxed);
        // join the thread, then the "this" objct, then call "loop()"
        worker_ = std::thread([this]
                              { loop(); });
    }

    ~AsyncWatcherDebouncer() { stop(); }

    void push_raw(const DebouncedEvent &e)
    {
        in_.enqueue(e);
    }

    void stop()
    {
        bool expected = false;

        if (stop_.compare_exchange_strong(expected, true, std::memory_order_relaxed))
        {
            std::cout << "stop" << std::endl;
            // the order is important here
            in_.enqueue({"/tmp/c.txt", DebouncedEventKind::Shutdown});
            if (worker_.joinable())
                worker_.join();
        }
    }

private:
    void loop()
    {
        DebouncedEvent ev;

        while (!stop_.load(std::memory_order_relaxed)) {
            if (!in_.wait_dequeue_timed(ev, delay_)) {
                continue;
            }

            if (ev.is_shutdown()) break;

            std::unordered_map<std::string, DebouncedEvent> bucket;


            bucket.emplace(ev.path, ev);

            const auto deadline = Clock::now() + delay_;

            // TODO: remove, temporary
            for (;;) {
                const auto now = Clock::now();
                if (now >= deadline) { break; }

                const auto left = duration_cast<milliseconds> (deadline - now);

                DebouncedEvent tmp;
                if (!in_.wait_dequeue_timed(tmp, left)) break;

                if (tmp.is_shutdown()) {
                    publish(bucket);
                    return;
                };


                bucket.insert_or_assign(tmp.path, std::move(tmp));
            }

            for (const auto& [key, ev]: bucket) {
               std::cout << key << " " << ev.path.string() << static_cast<int>(ev.kind) << std::endl;
            };


        }

    }

    void publish(std::unordered_map<std::string, DebouncedEvent>& bucket) {
        for (const auto& kv: bucket) {
            kv.second.kind == DebouncedEventKind::Shutdown ? void() :
            out_.send(kv.second);
        };
    };

    moodycamel::BlockingConcurrentQueue<DebouncedEvent> in_;
    milliseconds delay_;
    std::thread worker_;
    Sender out_;
    std::atomic<bool> stop_;
};

//int main()
//{


//    moodycamel::BlockingConcurrentQueue<int> q;
//    std::thread producer([&]()
//                         {
//        for (int i = 0; i != 100; ++i) {
//            std::this_thread::sleep_for(milliseconds(i % 10));
//            q.enqueue(i);
//        } });
//
//    std::thread consumer([&]()
//                         {
//        for (int i = 0; i != 100; ++i) {
//            // here the write happesn
//            int item;
//            q.wait_dequeue(item);
//            assert(item == i);
//
//            if (q.wait_dequeue_timed(item, milliseconds(5))) {
//                ++i;
//                assert(item == i);
//            }
//        } });
//    producer.join();
//    consumer.join();

    // assert(q.size_approx() == 0);
//    return 0;
//}

//
std::pair<std::unique_ptr<AsyncWatcherDebouncer>, Receiver>
 init_watcher(std::chrono::milliseconds delay = milliseconds(1500)) {
    auto concurrent_queue = std::make_shared<Queue>();
    Sender tx {concurrent_queue};
    Receiver rx {concurrent_queue};

    auto deb = std::make_unique<AsyncWatcherDebouncer>(delay, tx);

    return {std::move(deb), rx};
}

//// TODO: update this
#ifndef DEMO_MAIN

// всегда реферерс после типа стоит
//void push_file_update(Queue &q, const std::filesystem::path &p)
//{
//    q.enqueue({p, DebouncedEventKind::Update});
//}

 int main() {
    std::cout << "operational" << std::endl;

     auto [deb, rx] = init_watcher(std::chrono::milliseconds(1500));

     std::thread producer([&] {
         for (int i = 0; i < 5; ++i) {
            std::filesystem::path p1 = "/tmp/a.txt";
            std::filesystem::path p2 = "/tmp/b.txt";

            deb->push_raw({p1, DebouncedEventKind::Update});

            std::this_thread::sleep_for(milliseconds(100));

            deb->push_raw({p2, DebouncedEventKind::Update});
        }
    });

    std::thread consumer([&] {
        try {
            for (int i = 0; i < 3; ++i) {
                // write to this file  - this happens
                DebouncedEvent e;

                rx.recv(e);


                if (e.is_shutdown()) break;

                if (e.is_empty_payload()) {
                    throw std::runtime_error("debounced event has empty path");
                };

                std::cout << "debounced" << e.path << std::endl;
            };

        } catch (const std::exception& ex) {
            std::cerr << "consumer error " << std::endl;
        }
    });

    producer.join();
    deb->stop();
    consumer.join();

    return 0;
}

#endif
