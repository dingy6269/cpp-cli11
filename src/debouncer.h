#pragma once
#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <filesystem>
#include "blockingconcurrentqueue.h"

namespace app::debouncer {
using Clock = std::chrono::steady_clock;
using milliseconds = std::chrono::milliseconds;

enum class DebouncedEventKind {
    Update,
    Insert,
    Remove,
    Shutdown
};

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
    void publish(std::unordered_map<std::string, DebouncedEvent>& bucket);

    moodycamel::BlockingConcurrentQueue<DebouncedEvent> in_;
    milliseconds delay_;
    std::thread worker_;
    Sender out_;
    std::atomic<bool> stop_;
};

std::pair<std::unique_ptr<AsyncWatcherDebouncer>, Receiver>
init_watcher(std::chrono::milliseconds delay = milliseconds(1500));

} 