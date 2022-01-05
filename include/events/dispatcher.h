#pragma once

#include "event.h"

#include <map>
#include <vector>
#include <functional>

template<typename event_type>
class dispatcher {
    using callback_t = std::function<void(event_base<event_type> const &)>;
    std::multimap<event_type, callback_t> m_listeners;

    inline void on(event_type type, callback_t callback) {
        m_listeners.emplace(type, callback);
    }
public:
    template<event_type t>
    inline void emit(event<event_type, t> const &e) {
        auto callbacks = m_listeners.equal_range(t);
        for (auto it = callbacks.first; it != callbacks.second; it++)
            it->second(e);
    }

    template<typename T>
    inline void on(std::function<void(T const &e)> callback) {
        m_listeners.emplace(T::type_s(), [&](event_base<event_type> const &e) {
            T t = static_cast<T const &>(e);
            callback(t);
        });
    }
};
