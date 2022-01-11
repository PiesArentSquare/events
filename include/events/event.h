#pragma once

#include "network/message.h"

namespace events {

template<typename event_type>
class event_base {
protected:
    template<typename... Args>
    inline void serialize_impl(message &msg, Args&... args) const {
        (msg << ... << args);
    }

    template<typename... Args>
    inline void deserialize_impl(message &msg, Args&... args) {
        (msg >> ... >> args);
    }

    template<typename... Args>
    inline constexpr size_t get_fields_size(Args...) const {
        return (... + sizeof(Args));
    }
public:
    virtual ~event_base() = default;
    virtual event_type type() const = 0;
    
    virtual void serialize(message &msg) const {}
    virtual void deserialize(message &msg) {}
    virtual constexpr size_t byte_size() const { return 0; };
};

template<typename event_type, event_type t>
class event : public event_base<event_type> {
protected:
    event() = default;
public:
    virtual ~event() = default;

    inline static constexpr event_type type_s() { return t; }
    inline event_type type() const override { return t; };
};

#define EVENTS__SET_FIELDS(...) \
    void serialize(::events::message &msg) const override { serialize_impl(msg, __VA_ARGS__); } \
    void deserialize(::events::message &msg) override { deserialize_impl(msg, __VA_ARGS__); } \
    constexpr size_t byte_size() const override { return get_fields_size(__VA_ARGS__); } \

}