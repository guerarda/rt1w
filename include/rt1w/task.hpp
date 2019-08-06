#pragma once

#include "rt1w/sptr.hpp"

struct Event;

struct Task : Object {
    virtual sptr<Event> schedule() = 0;
};
