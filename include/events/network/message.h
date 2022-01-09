#pragma once

#include <cstring>
#include <cstdint>
#include <vector>
#include <stdexcept>

namespace events {

template<typename event_type>
class connection;

class message {
    std::vector<uint8_t> m_body;
    std::vector<uint8_t>::const_iterator m_it;

    template<typename> friend class connection;
public:
    message() : m_it(m_body.begin()) {};
    // message(message const &msg) : m_body(msg.m_body), m_it(m_body.begin()) {}
    // message(message &&msg) : m_body(std::move(msg.m_body)), m_it(m_body.begin()) {}
    
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

    void resize(size_t size) {
        m_body.resize(size);
        m_it = m_body.begin();
    }
};

}
