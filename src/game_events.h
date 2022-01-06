#pragma once
#include <events/event.h>

enum class game_events {
    my_event
};

class my_event : public events::event<game_events, game_events::my_event> {
    int m_i;
public:
    explicit my_event(int i) : m_i(i) {}
    my_event() = default;

    void serialize(events::message &msg) const {
        msg << m_i;
    };
    void deserialize(events::message &msg) {
        msg >> m_i;
    };

    int get_i() const { return m_i; }
};
