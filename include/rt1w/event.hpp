#pragma once

#include "rt1w/sptr.hpp"
#include "rt1w/types.h"
#include "rt1w/workq.hpp"

#include <vector>

struct Event : Object {
    static sptr<Event> create(int32_t);
    static sptr<Event> create(const std::vector<sptr<Event>> &events);

    virtual sptr<Event> notify(workq *workq,
                               workq_func func,
                               const sptr<Object> &obj,
                               const sptr<Object> &arg) = 0;
    virtual int32_t signal() = 0;
    virtual bool test() const = 0;
    virtual int32_t wait() = 0;
};
