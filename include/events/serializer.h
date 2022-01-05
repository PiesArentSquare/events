#pragma once

#include <cstring>
#include <string>
#include <vector>

namespace events {

class message {
    std::vector<uint8_t> m_body;
public:
    template<typename T, typename = std::enable_if_t<std::is_trivial_v<T>>>
    message operator<<(T const &t) {
        size_t i = m_body.size();
        m_body.resize(i + sizeof(T));
        std::memcpy(m_body.data() + i, &t, sizeof(T));
        return *this;
    }

    template<typename T, typename = std::enable_if_t<std::is_trivial_v<T>>>
    message operator>>(T &t) {
        size_t i = m_body.size() - sizeof(T);
        std::memcpy(&t, m_body.data() + i, sizeof(T));
        m_body.resize(i);
        return *this;
    }
};

}
