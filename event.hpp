#ifndef EVENT_H
#define EVENT_H

#include "types.h"
#include "sptr.hpp"

struct event {

    static sptr<event> create(uint32_t n);

    virtual int32_t signal() = 0;
    virtual bool test() const = 0;
    virtual int32_t wait() = 0;
};

#endif
