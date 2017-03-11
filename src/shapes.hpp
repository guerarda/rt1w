#ifndef SHAPES_H
#define SHAPES_H

#include "hitable.hpp"

struct XY_Rect : Hitable {
    static sptr<Hitable> create(float x0, float x1,
                                float y0, float y1,
                                float z,
                                const sptr<Material> &mat);
};

#endif
