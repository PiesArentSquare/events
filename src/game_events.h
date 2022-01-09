#pragma once
#include <events/event.h>
#include <chrono>

enum class game_events : uint32_t {
    ping_server = 5
};

class ping_server : public events::event<game_events, game_events::ping_server> {
    std::chrono::system_clock::time_point m_now;
public:
    explicit ping_server(std::chrono::system_clock::time_point now) : m_now(now) {}
    ping_server() = default;

    void serialize(events::message &msg) const override {
        msg << m_now;
    };

    void deserialize(events::message &msg) override {
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
