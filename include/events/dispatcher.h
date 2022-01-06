#pragma once

#include "event.h"

#include <map>
#include <vector>
#include <functional>

namespace events {

template<typename event_type>
class dispatcher {
    using callback_t = std::function<void(event_base<event_type> const &)>;
    std::multimap<event_type, callback_t> m_listeners;
public:
    inline void emit(event_base<event_type> const &e) {
        auto callbacks = m_listeners.equal_range(e.type());
        for (auto it = callbacks.first; it != callbacks.second; it++)
            it->second(e);
    }

    template<typename T>
    inline void on(std::function<void(T const &e)> const &callback) {
        m_listeners.emplace(T::type_s(), [&callback](event_base<event_type> const &e) {
            T const &te = static_cast<T const &>(e);
            callback(te);
        });
    }
};

}
