#pragma once

#include "sptr.hpp"
#include "types.h"
#include "workq.hpp"

struct Event : Object {
    static sptr<Event> create(int32_t);

    virtual int32_t notify(workq *workq,
                           workq_func func,
                           const sptr<Object> &obj,
                           const sptr<Object> &arg) = 0;
    virtual int32_t signal() = 0;
    virtual bool test() const = 0;
    virtual int32_t wait() = 0;
};
