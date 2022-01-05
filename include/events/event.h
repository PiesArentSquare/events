#pragma once

namespace events {

template<typename event_type>
class event_base {
public:
    virtual ~event_base() = default;
    virtual event_type type() const = 0;
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