#ifndef EVENT_H
#define EVENT_H

#include "types.h"
#include "sptr.hpp"
#include "wqueue.hpp"

struct event : Object {

    static sptr<event> create(int32_t);

    virtual int32_t notify(wqueue *wqueue,
                           wqueue_func func,
                           const sptr<Object> &obj,
                           const sptr<Object> &arg) = 0;
    virtual int32_t signal() = 0;
    virtual bool test() const = 0;
    virtual int32_t wait() = 0;
};

#endif
