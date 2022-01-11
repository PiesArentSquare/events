#pragma once
#include <events/event.h>
#include <chrono>
#include <iostream>

enum class game_events : uint32_t {
    server_accept,
    ping_server,
    message_all,
    server_message
};

class server_accept : public events::event<game_events, game_events::server_accept> { };

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
        return sizeof(m_now);
    }

    auto get_now() const { return m_now; }
};

class message_all : public events::event<game_events, game_events::message_all> { };

class server_message : public events::event<game_events, game_events::server_message> {
    uint32_t m_sender_id;
public:
    explicit server_message(uint32_t sender_id) : m_sender_id(sender_id) {}
    server_message() = default;

    void serialize(events::message &msg) const override {
        msg << m_sender_id;
    }

    void deserialize(events::message &msg) override {
        msg >> m_sender_id;
    }

    size_t byte_size() const override {
        return sizeof(m_sender_id);
    }

    auto get_sender_id() const { return m_sender_id; }
};

events::event_base<game_events> *mapper(game_events t) {
    switch(t) {
        case game_events::ping_server:
            return new ping_server;
        case game_events::message_all:
            return new message_all;
        case game_events::server_message:
            return new server_message;
        case game_events::server_accept:
            return new server_accept;
    }
    return nullptr;
}
