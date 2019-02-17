#pragma once

#include "sptr.hpp"

struct Event;

struct Task : Object {
    virtual sptr<Event> schedule() = 0;
};
