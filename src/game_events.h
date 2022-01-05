#pragma once
#include <events/event.h>

enum class game_events {
    my_event
};

class my_event : public event<game_events, game_events::my_event> {
    int m_i;
public:
    explicit my_event(int i) : m_i(i) {}
    my_event() = default;

    pson::Json serialize() const override {
        pson::Json output;
        output->addProperty("m_i", m_i);
        return output;
    }

    void deserialize(pson::Json input) override {
        m_i = input->getAsInt("m_i");
    }

    int get_i() const { return m_i; }
};