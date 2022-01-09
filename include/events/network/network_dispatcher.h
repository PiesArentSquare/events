#pragma once

#include "../event.h"

#include <map>
#include <functional>
#include <memory>

namespace events {

template<typename event_type>
class connection;

template<typename event_type>
using mapper_func = std::function<event_base<event_type> *(event_type)>;
template<typename event_type>
using remote_connection = std::shared_ptr<connection<event_type>>;

template<typename event_type>
class network_dispatcher {
    std::multimap<event_type, std::function<void(event_base<event_type> const &, remote_connection<event_type>)>> m_listeners;
    mapper_func<event_type> m_mapper;

    event_base<event_type> *get_event(event_type t) {
        return m_mapper(t);
    }
public:
    explicit network_dispatcher(mapper_func<event_type> mapper) : m_mapper(mapper) {}

    friend connection<event_type>;
    inline void emit(event_base<event_type> const &e, remote_connection<event_type> remote) {
        auto callbacks = m_listeners.equal_range(e.type());
        for (auto it = callbacks.first; it != callbacks.second; it++)
            it->second(e, remote);
    }

    template<typename T>
    inline void on(std::function<void(T const &e, remote_connection<event_type> remote)> const &callback) {
        m_listeners.emplace(T::type_s(), [callback](event_base<event_type> const &e, remote_connection<event_type> remote) {
            T const &te = static_cast<T const &>(e);
            callback(te, remote);
        });
    }

    template<typename T>
    inline void on(std::function<void(T const &e)> const &callback) {
        m_listeners.emplace(T::type_s(), [callback](event_base<event_type> const &e, remote_connection<event_type> remote) {
            T const &te = static_cast<T const &>(e);
            callback(te);
        });
    }
};

}
