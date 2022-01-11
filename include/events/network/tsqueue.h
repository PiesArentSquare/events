#pragma once

#include <deque>
#include <mutex>
#include <condition_variable>

namespace events {

template<typename T>
class tsqueue {
    std::mutex m_queue_lock;
    std::deque<T> m_queue;

    std::condition_variable m_blocking;
    std::mutex m_block_lock;
public:
    tsqueue() = default;
    tsqueue(tsqueue<T> const &) = delete;
    ~tsqueue() { clear(); }

    T &front() {
        std::scoped_lock lock(m_queue_lock);
        return m_queue.front();
    }

    T &back() {
        std::scoped_lock lock(m_queue_lock);
        return m_queue.back();
    }

    void push_front(T const &item) {
        std::scoped_lock lock(m_queue_lock);
        m_queue.emplace_front(item);

        std::unique_lock<std::mutex> block_lock(m_block_lock);
        m_blocking.notify_one();
    }

    void push_back(T const &item) {
        std::scoped_lock lock(m_queue_lock);
        m_queue.emplace_back(item);

        std::unique_lock<std::mutex> block_lock(m_block_lock);
        m_blocking.notify_one();
    }

    [[nodiscard]] bool is_empty() {
        std::scoped_lock lock(m_queue_lock);
        return m_queue.empty();
    }

    size_t count() {
        std::scoped_lock lock(m_queue_lock);
        return m_queue.size();
    }

    void clear() {
        std::scoped_lock lock(m_queue_lock);
        m_queue.clear();
    }

    T pop_front() {
        std::scoped_lock lock(m_queue_lock);
        auto t = std::move(m_queue.front());
        m_queue.pop_front();
        return t;
    }

    T pop_back() {
        std::scoped_lock lock(m_queue_lock);
        auto t = std::move(m_queue.back());
        m_queue.pop_back();
        return t;
    }

    void wait() {
        while (is_empty()) {
            std::unique_lock<std::mutex> lock(m_block_lock);
            m_blocking.wait(lock);
        }
    }
};

}