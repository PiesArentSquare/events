#pragma once
#include <events/event.h>
#include <events/serializer.h>

enum class game_events {
    my_event
};

class my_event : public events::event<game_events, game_events::my_event> {
    int m_i;
public:
    explicit my_event(int i) : m_i(i) {}
    my_event() = default;

    int get_i() const { return m_i; }
};
