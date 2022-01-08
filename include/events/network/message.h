#pragma once

#include <cstring>
#include <cstdint>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <memory>

namespace events {

template<typename event_type>
class connection;

template<typename event_type>
class message {
    event_type m_type;
    std::vector<uint8_t> m_body;
    std::vector<uint8_t>::const_iterator m_it;

    friend class connection<event_type>;
    
    message() : m_it(m_body.begin()) {}
public:
    explicit message(event_type type) : m_type(type), m_it(m_body.begin()) {};
    message(message const &msg) : m_type(msg.m_type), m_body(msg.m_body), m_it(m_body.begin()) {}
    message(message &&msg) : m_type(std::move(msg.m_type)), m_body(std::move(msg.m_body)), m_it(m_body.begin()) {}
    
    template<typename T, typename = std::enable_if_t<std::is_standard_layout_v<T>>>
    inline message &operator<<(T const &t) {
        size_t i = m_body.size();
        m_body.resize(i + sizeof(T));
        std::memcpy(m_body.data() + i, &t, sizeof(T));
        m_it = m_body.begin();
        return *this;
    }

    template<typename T, typename = std::enable_if_t<std::is_standard_layout_v<T>>>
    inline message &operator>>(T &t) {
        if (m_it + sizeof(T) > m_body.end()) throw std::runtime_error("attempted to read out of bounds");
        std::memcpy(&t, &(*m_it), sizeof(T));
        m_it += sizeof(T);
        return *this;
    }

    inline event_type type() const { return m_type; }
};

template<typename event_type>
struct owned_message {
    message<event_type> msg;
    std::shared_ptr<connection<event_type>> remote = nullptr;
};

}
