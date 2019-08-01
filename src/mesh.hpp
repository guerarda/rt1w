#pragma once

#include "geometry.hpp"
#include "shape.hpp"
#include "sptr.hpp"

#include <vector>

struct Params;
struct Transform;
struct Triangle;

struct VertexData : Object {
    static sptr<VertexData> create(size_t nv,
                                   uptr<std::vector<v3f>> &v,
                                   uptr<std::vector<v3f>> &n,
                                   uptr<std::vector<v2f>> &uv);
    friend Triangle;

protected:
    VertexData(size_t nv, const v3f *v, const v3f *n, const v2f *uv) :
        m_nv(nv),
        m_v(v),
        m_n(n),
        m_uv(uv)
    {}

    const size_t m_nv;
    const v3f *m_v;
    const v3f *m_n;
    const v2f *m_uv;
};

struct Mesh : Group {
    static sptr<Mesh> create(const sptr<Params> &p);
    static sptr<Mesh> create(size_t nt,
                             const sptr<VertexData> &vd,
                             const sptr<const std::vector<uint32_t>> &i,
                             const Transform &worldToObj);
    static sptr<Mesh> create(size_t nt,
                             uptr<std::vector<v3f>> &v,
                             uptr<std::vector<v3f>> &n,
                             uptr<std::vector<v2f>> &uv,
                             const sptr<const std::vector<uint32_t>> &i,
                             const Transform &worldToObj);
    virtual std::vector<sptr<Shape>> faces() const = 0;
};
