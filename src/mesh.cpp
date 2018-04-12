#include "mesh.hpp"
#include "params.hpp"
#include "error.h"
#include "value.hpp"
#include "ray.hpp"
#include "primitive.hpp"
#include "utils.h"

#pragma mark - Vertex Data

struct _VertexData : VertexData {
    _VertexData(size_t nv,
                uptr<v3f[]> &v,
                uptr<v3f[]> &n,
                uptr<v2f[]> &uv) : VertexData(nv, v, n , uv) { }
};


sptr<VertexData> VertexData::create(size_t nv,
                                    uptr<v3f[]> &v,
                                    uptr<v3f[]> &n,
                                    uptr<v2f[]> &uv)
{
    return std::make_shared<_VertexData>(nv, v, n, uv);
}

#pragma mark - Mesh Data

struct MeshData {

    MeshData(size_t nt,
             const sptr<VertexData> &vd,
             uptr<uint32_t[]> &i) : m_nt(nt), m_vd(vd), m_i(std::move(i)) { }

    size_t           m_nt;
    sptr<VertexData> m_vd;
    uptr<uint32_t[]> m_i;
};

#pragma mark - Triangle

struct Triangle : Shape {

    Triangle(const sptr<const MeshData> &md, size_t ix) : m_md(md),
                                                          m_v(&md->m_i[3 * ix]) { }

    bool     hit(const sptr<ray> &, float, float, hit_record &) const override;
    bounds3f bounds() const override;

    sptr<const MeshData>   m_md;
    const uint32_t * const m_v;
};

static v3f absv(const v3f &v)
{
    return { std::abs(v.x), std::abs(v.y), std::abs(v.z) };
}

static size_t max_dimension(const v3f &v)
{
    return v.x > v.y ? (v.x > v.z ? 0 : 2) : (v.y > v.z ? 1 : 2);
}

bool Triangle::hit(const sptr<ray> &r, float min, float max, hit_record &rec) const
{
    sptr<VertexData> vd = m_md->m_vd;

    /* Get triangle coordinates */
    v3f p0 = vd->m_v[m_v[0]];
    v3f p1 = vd->m_v[m_v[1]];
    v3f p2 = vd->m_v[m_v[2]];

    /* Transform triangle into ray-space coordinates */
    v3f p0t = p0 - r->origin();
    v3f p1t = p1 - r->origin();
    v3f p2t = p2 - r->origin();

    /* Permute */
    size_t kz = max_dimension(absv(r->direction()));
    size_t kx = (kz + 1) % 3;
    size_t ky = (kx + 1) % 3;

    v3f d = { r->direction()[kx], r->direction()[ky], r->direction()[kz] };

    p0t = { p0t[kx], p0t[ky], p0t[kz] };
    p1t = { p1t[kx], p1t[ky], p1t[kz] };
    p2t = { p2t[kx], p2t[ky], p2t[kz] };

    /* Shear */
    float Sx = -d.x / d.z;
    float Sy = -d.y / d.z;
    float Sz = 1 / d.z;

    p0t.x += Sx * p0t.z;
    p0t.y += Sy * p0t.z;
    p0t.z *= Sz;

    p1t.x += Sx * p1t.z;
    p1t.y += Sy * p1t.z;
    p1t.z *= Sz;

    p2t.x += Sx * p2t.z;
    p2t.y += Sy * p2t.z;
    p2t.z *= Sz;

    /* Edge functions */
    float e0 = (float)((double)p1t.x * (double)p2t.y - (double)p1t.y * (double)p2t.x);
    float e1 = (float)((double)p2t.x * (double)p0t.y - (double)p2t.y * (double)p0t.x);
    float e2 = (float)((double)p0t.x * (double)p1t.y - (double)p0t.y * (double)p1t.x);

    if (   (e0 < 0.0f || e1 < 0.0f || e2 < 0.0f)
           && (e0 > 0.0f || e1 > 0.0f || e2 > 0.0f)) {
        return false;
    }

    float det = e0 + e1 + e2;
    if (f32_equal(det, 0.0f)) {
        return false;
    }

    /* Value of parameter t & early rejections */
    float t = e0 * p0t.z + e1 * p1t.z + e2 * p2t.z;
    if (det > 0 && (t <= min * det || t >= max * det)) {
        return false;
    }
    if (det < 0 && (t >= min * det || t <= max * det)) {
        return false;
    }

    /* Barycentric coordinates */
    float idet = 1.0f / det;
    float b0 = e0 * idet;
    float b1 = e1 * idet;
    float b2 = e2 * idet;
    t *= idet;

    /* Texture coordinates */
    v2f uv0 = { 0.0, 0.0 };
    v2f uv1 = { 0.0, 1.0 };
    v2f uv2 = { 1.0, 1.0 };

    if (vd->m_uv) {
        uv0 = vd->m_uv[m_v[0]];
        uv1 = vd->m_uv[m_v[1]];
        uv2 = vd->m_uv[m_v[2]];
    }

    /* Normals */
    v3f dp02 = p0 - p2;
    v3f dp12 = p1 - p2;
    v3f n = Normalize(Cross(dp02, dp12));

    /* Update record */
    rec.t = t;
    rec.p = b0 * p0 + b1 * p1 + b2 * p2;
    rec.uv = b0 * uv0 + b1 * uv1 + b2 * uv2;
    rec.normal = n;

    return true;
}

bounds3f Triangle::bounds() const
{
    sptr<VertexData> vd = m_md->m_vd;

    const v3f &p0 = vd->m_v[m_v[0]];
    const v3f &p1 = vd->m_v[m_v[1]];
    const v3f &p2 = vd->m_v[m_v[2]];

    return Union(bounds3f(p0, p1), p2);
}

#pragma mark - Mesh

struct _Mesh : Mesh {
    _Mesh(const sptr<MeshData> &md);

    bool     hit(const sptr<ray> &, float, float, hit_record &) const override;
    bounds3f bounds() const override { return m_box; };

    std::vector<sptr<Shape>> faces() const override;

    bounds3f                    m_box;
    sptr<MeshData>              m_md;
    std::vector<sptr<Triangle>> m_tris;
};

_Mesh::_Mesh(const sptr<MeshData> &md)
{
    m_md = md;

    m_tris.reserve(md->m_nt);
    for (size_t i = 0; i < md->m_nt; i++) {
        sptr<Triangle> t = std::make_shared<Triangle>(md, i);
        m_tris.push_back(t);
        m_box = Union(m_box, t->bounds());
    }
}

bool _Mesh::hit(const sptr<ray> &r, float min, float max, hit_record &rec) const
{
    for (auto &t : m_tris) {
        if (t->hit(r, min, max, rec)) {
            return true;
        }
    }
    return false;
}

std::vector<sptr<Shape>> _Mesh::faces() const
{
    return std::vector<sptr<Shape>>(m_tris.begin(), m_tris.end());
}

#pragma mark - Static Constructors

sptr<Mesh> Mesh::create(size_t nt,
                        const sptr<VertexData> &vd,
                        const sptr<Value> &indices)
{
    uptr<uint32_t[]> i = std::make_unique<uint32_t[]>(3 * nt);
    indices->value(TYPE_UINT32, i.get(), 0, 3 * nt);

    sptr<MeshData> md = std::make_shared<MeshData>(nt, vd, i);

    return std::make_shared<_Mesh>(md);
}

sptr<Mesh> Mesh::create(size_t nt,
                        const sptr<Value> &vertices,
                        const sptr<Value> &indices,
                        const sptr<Value> &normals,
                        const sptr<Value> &uvs)
{
    ASSERT(vertices);
    ASSERT(indices);

    size_t nv = vertices->count() / 3;

    uptr<v3f[]> v = std::make_unique<v3f[]>(nv);
    vertices->value(TYPE_FLOAT32, v.get(), 0, 3 * nv);

    uptr<v3f[]> n;
    if (normals) {
        n = std::make_unique<v3f[]>(3 * nv);
        normals->value(TYPE_FLOAT32, n.get(), 0, 3 * nv);
    }
    uptr<v2f[]> uv;
    if (uv) {
        uv = std::make_unique<v2f[]>(2 * nv);
        uvs->value(TYPE_FLOAT32, uv.get(), 0, 2 * nv);
    }

    sptr<VertexData> vd = VertexData::create(nv, v, n, uv);

    return Mesh::create(nt, vd, indices);
}

sptr<Mesh> Mesh::create(const sptr<Params> &p)
{
    sptr<Value> c  = p->value("count");
    sptr<Value> v  = p->value("vertices");
    sptr<Value> n  = p->value("normals");
    sptr<Value> uv = p->value("uv");
    sptr<Value> i  = p->value("indices");

    if (c && v && i) {
        size_t nt = (size_t)c->u64();
        bool ok_i = 3 * nt == i->count();
        bool ok_n = n ? n->count() == v->count() : true;
        bool ok_uv = uv ? 2 * v->count() == 3 * uv->count() : true;

        if (ok_i && ok_n && ok_uv) {
            return Mesh::create(nt, v, i, n, uv);
        }
        WARNING_IF(!ok_i,  "Triangle count doesn't match index count");
        WARNING_IF(!ok_n,  "Normal count doesn't match vertex count");
        WARNING_IF(!ok_uv, "UV count doesn't match vertex count");
    }
    ERROR_IF(!c, "Mesh parameter \"count\" not specified");
    ERROR_IF(!v, "Mesh parameter \"vertices\" not specified");
    ERROR_IF(!i, "Mesh parameter \"indices\" not specified");
    WARNING_IF(!n, "Mesh parameter \"normals\" not specified");
    WARNING_IF(!uv,"Mesh parameter \"uv\" not specified");

    return nullptr;
}
