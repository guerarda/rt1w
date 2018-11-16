#pragma once

#include "sptr.hpp"
#include "types.h"

#include <string>

struct Params;

struct Image : Object {

    static sptr<Image> create(const std::string &filename);
    static sptr<Image> create(const sptr<Params> &p);

    virtual buffer_t buffer() const = 0;
    virtual v2u      size() const = 0;
};
