#pragma once
#include "dispatcher.h"

#include <functional>
#include <iostream>

namespace events {

template<typename event_type>
class network_dispatcher;

template<typename event_type>
class connection {
    network_dispatcher<event_type> &m_dispatcher;
public:
    connection(network_dispatcher<event_type> &dispatcher) : m_dispatcher(dispatcher) {}

    void send(event_type t, message const &msg) const {
        m_dispatcher.receive(t, msg);
    }
};

template<typename event_type>
class network_dispatcher : protected dispatcher<event_type> {
    using mapper_func = std::function<event_base<event_type>*(event_type)>;
    mapper_func m_mapper;

    friend connection<event_type>;
    inline void receive(event_type t, message msg) {
        event_base<event_type> *e = m_mapper(t);
        e->deserialize(msg);
        dispatcher<event_type>::emit(*e);
        delete e;
    }
public:
    network_dispatcher(mapper_func mapper) : m_mapper(mapper) {}

    inline void emit(event_base<event_type> const &e, connection<event_type> const &c) {
        message msg;
        e.serialize(msg);
        c.send(e.type(), msg);
    }

    template<typename T>
    inline void on(std::function<void(T const &e)> const &callback) {
        dispatcher<event_type>::on(callback);
    }
};

}
