#pragma once

#include "message.h"

namespace events {

template<typename event_type>
class event_base {
public:
    virtual ~event_base() = default;
    virtual event_type type() const = 0;
    
    virtual void serialize(message &msg) const {}
    virtual void deserialize(message &msg) {}
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

}