#pragma once

#include "rt1w/geometry.hpp"
#include "rt1w/sptr.hpp"
#include "rt1w/task.hpp"
#include "rt1w/types.h"

#include <string>

struct Params;

struct Image : Task {
    static sptr<Image> create(const std::string &filename);
    static sptr<Image> create(const sptr<Image> &img, buffer_format_t fmt);
    static sptr<Image> create(const sptr<Params> &p);

    virtual buffer_t buffer() = 0;
    virtual v2u size() const = 0;
};
