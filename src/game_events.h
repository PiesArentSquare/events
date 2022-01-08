#pragma once
#include <events/event.h>
#include <chrono>

enum class game_events : uint32_t {
    // my_event,
    ping_server = 5
};

// class my_event : public events::event<game_events, game_events::my_event> {
//     int m_i;
// public:
//     explicit my_event(int i) : m_i(i) {}
//     my_event() = default;

//     void serialize(events::message<game_events> &msg) const override {
//         msg << m_i;
//     };

//     void deserialize(events::message<game_events> &msg) override {
//         msg >> m_i;
//     };

//     int get_i() const { return m_i; }
// };

class ping_server : public events::event<game_events, game_events::ping_server> {
    std::chrono::system_clock::time_point m_now;
public:
    explicit ping_server(std::chrono::system_clock::time_point now) : m_now(now) {}
    ping_server() = default;

    void serialize(events::message<game_events> &msg) const override {
        msg << m_now;
    };

    void deserialize(events::message<game_events> &msg) override {
        msg >> m_now;
    };

    size_t byte_size() const override {
        return events::get_member_size(m_now);
    }

    auto get_now() const { return m_now; }
};

events::event_base<game_events> *mapper(game_events t) {
    switch(t) {
        case game_events::ping_server:
            return new ping_server;
    }
    return nullptr;
}
