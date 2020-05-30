#pragma once

#include "rt1w/sptr.hpp"

#include <vector>

struct Event;

struct Task : Object {
    virtual sptr<Event> schedule() = 0;
};

template <typename T>
struct Batch : Task {
    virtual const std::vector<T> &content() = 0;
};
