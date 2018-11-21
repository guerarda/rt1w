#pragma once

#include <vector>

#include "shape.hpp"
#include "sptr.hpp"

struct Params;
struct Value;

struct VertexData : Object {
    static sptr<VertexData> create(size_t nv,
                                   uptr<v3f[]> &v,
                                   uptr<v3f[]> &n,
                                   uptr<v2f[]> &uv);
    const size_t m_nv;
    const uptr<const v3f[]> m_v;
    const uptr<const v3f[]> m_n;
    const uptr<const v2f[]> m_uv;

protected:
    VertexData(size_t nv, uptr<v3f[]> &v, uptr<v3f[]> &n, uptr<v2f[]> &uv) :
        m_nv(nv),
        m_v(std::move(v)),
        m_n(std::move(n)),
        m_uv(std::move(uv))
    {}
};

struct Mesh : Shape {
    static sptr<Mesh> create(size_t nt,
                             const sptr<Value> &vertices,
                             const sptr<Value> &indices,
                             const sptr<Value> &normals,
                             const sptr<Value> &uvs);

    static sptr<Mesh> create(size_t nt,
                             const sptr<VertexData> &vd,
                             const sptr<Value> &indices);

    static sptr<Mesh> create(const sptr<Params> &params);

    virtual std::vector<sptr<Shape>> faces() const = 0;
};
