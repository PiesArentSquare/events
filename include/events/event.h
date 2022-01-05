#pragma once

#include <sstream>

#include <pson/json.h>

template<typename event_type>
class event_base {
public:
    virtual ~event_base() = default;
    virtual event_type type() const = 0;
    virtual pson::Json serialize() const = 0;
    virtual void deserialize(pson::Json input) = 0;
};

template<typename event_type, event_type t>
class event : public event_base<event_type> {
public:
    virtual ~event() = default;

    inline static constexpr event_type type_s() { return t; }
    inline event_type type() const override { return t; };
};