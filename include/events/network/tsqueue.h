#pragma once

#include <mutex>
#include <deque>

namespace events {

template<typename T>
class tsqueue {
// protected:
    std::mutex m_mux;
    std::deque<T> m_queue;
public:
    tsqueue() = default;
    tsqueue(tsqueue<T> const &) = delete;
    /* virtual */ ~tsqueue() { clear(); }

    T const &front() {
        std::scoped_lock lock(m_mux);
        return m_queue.front();
    }

    T const &back() {
        std::scoped_lock lock(m_mux);
        return m_queue.back();
    }

    void push_front(T const &item) {
        std::scoped_lock lock(m_mux);
        m_queue.emplace_front(item);
    }

    void push_back(T const &item) {
        std::scoped_lock lock(m_mux);
        m_queue.emplace_back(item);
    }

    [[nodiscard]] bool is_empty() {
        std::scoped_lock lock(m_mux);
        return m_queue.empty();
    }

    size_t count() {
        std::scoped_lock lock(m_mux);
        return m_queue.size();
    }

    void clear() {
        std::scoped_lock lock(m_mux);
        m_queue.clear();
    }

    T pop_front() {
        std::scoped_lock lock(m_mux);
        auto t = std::move(m_queue.front());
        m_queue.pop_front();
        return t;
    }

    T pop_back() {
        std::scoped_lock lock(m_mux);
        auto t = std::move(m_queue.back());
        m_queue.pop_back();
        return t;
    }
};

}