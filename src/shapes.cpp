#include "shapes.hpp"
#include <cmath>
#include <cfloat>

#define BOX_HALF_WIDTH 0.0001f

struct _XY_Rect : XY_Rect {

    _XY_Rect(float x0, float x1,
             float y0, float y1,
             float z,
             const sptr<Material> &mat);

    bool  hit(const sptr<ray> &r, float min, float max, hit_record &rec) const;
    box_t bounding_box() const { return m_box; }


    box_t m_box;
    float m_z;
    sptr<Material> m_mat;
};

_XY_Rect::_XY_Rect(float x0, float x1,
                   float y0, float y1,
                   float z,
                   const sptr<Material> &mat)
{
    m_box.lo.x = fminf(x0, x1);
    m_box.lo.y = fminf(y0, y1);
    m_box.lo.z = z - BOX_HALF_WIDTH;

    m_box.hi.x = fmaxf(x0, x1);
    m_box.hi.y = fmaxf(y0, y1);
    m_box.hi.z = z + BOX_HALF_WIDTH;

    m_z = z;
    m_mat = mat;
}

static int32_t f32_cmp(float x, float y)
{
    if (fabs(x - y) <= fmaxf(fabsf(x), fabsf(y)) * FLT_EPSILON) {
        return 0;
    } else {
        return x < y ? -1 : + 1;
    }
}

bool _XY_Rect::hit(const sptr<ray> &r, float min, float max, hit_record &rec) const
{
    float t = (m_z - r->origin().z) / r->direction().z;

    if (f32_cmp(t, min) > 0 && f32_cmp(t, max) < 0) {
        float x = r->origin().x + t * r->direction().x;
        float y = r->origin().y + t * r->direction().y;
        float x0 = m_box.lo.x;
        float x1 = m_box.hi.x;
        float y0 = m_box.lo.y;
        float y1 = m_box.hi.y;

        if (   f32_cmp(x, x0) > 0
            && f32_cmp(y, y0) > 0
            && f32_cmp(x, x1) < 0
            && f32_cmp(y, y1) < 0) {

            rec.p = r->point(t);
            rec.uv.x = (x - x0) / (x1 - x0);
            rec.uv.y = (y - y0) / (y1 - y0);
            rec.normal = { 0.0f, 0.0f, 1.0f };
            rec.mat = m_mat;

            return true;
        }
    }
    return false;
}

#pragma mark - Static constructors

sptr<Hitable> XY_Rect::create(float x0, float x1,
                              float y0, float y1,
                              float z,
                              const sptr<Material> &mat)
{
    return std::make_shared<_XY_Rect>(x0, x1, y0, y1, z, mat);
}
