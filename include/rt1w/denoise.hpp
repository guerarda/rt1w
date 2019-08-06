#pragma once

#include "rt1w/sptr.hpp"

struct Image;

sptr<Image> Denoise(const sptr<Image> &color);
sptr<Image> Denoise(const sptr<Image> &color,
                    const sptr<Image> &normals,
                    const sptr<Image> &albedo);
