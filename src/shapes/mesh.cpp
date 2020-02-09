#include "shapes/mesh-priv.hpp"

#include "rt1w/error.h"
#include "rt1w/interaction.hpp"
#include "rt1w/params.hpp"
#include "rt1w/primitive.hpp"
#include "rt1w/ray.hpp"
#include "rt1w/sampling.hpp"
#include "rt1w/shape.hpp"
#include "rt1w/transform.hpp"
#include "rt1w/utils.hpp"
#include "rt1w/value.hpp"

#pragma mark - Triangle

struct Triangle : Shape {
    Triangle(const sptr<MeshData> &md, size_t ix) : m_md(md), m_v(&(md->m_i[3 * ix])) {}

    bool intersect(const Ray &r, Interaction &isect) const override;
    bool qIntersect(const Ray &r) const override;

    float area() const override;
    bounds3f bounds() const override;
    Transform worldToObj() const override { return m_md->m_worldToObj; }

    Interaction sample(const v2f &u) const override;
    float pdf() const override { return 1.f / area(); }

    Interaction sample(const Interaction &ref, const v2f &u) const override;
    float pdf(const Interaction &ref, const v3f &wi) const override;

    sptr<const MeshData> m_md;
    const uint32_t *const m_v;
};

static size_t MaxDimension(const v3f &v)
{
    return v.x > v.y ? (v.x > v.z ? 0 : 2) : (v.y > v.z ? 1 : 2);
}

bool Triangle::intersect(const Ray &ray, Interaction &isect) const
{
    sptr<VertexData> vd = m_md->m_vd;

    /* Get triangle coordinates */
    v3f p0 = vd->m_v[m_v[0]];
    v3f p1 = vd->m_v[m_v[1]];
    v3f p2 = vd->m_v[m_v[2]];

    /* Transform to object space */
    Ray r = m_md->m_worldToObj(ray);

    /* Transform triangle into ray-space coordinates */
    v3f p0t = p0 - r.org();
    v3f p1t = p1 - r.org();
    v3f p2t = p2 - r.org();

    /* Permute */
    size_t kz = MaxDimension(Abs(r.dir()));
    size_t kx = (kz + 1) % 3;
    size_t ky = (kx + 1) % 3;

    v3f d = { r.dir()[kx], r.dir()[ky], r.dir()[kz] };

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
    auto e0 = p1t.x * p2t.y - p1t.y * p2t.x;
    auto e1 = p2t.x * p0t.y - p2t.y * p0t.x;
    auto e2 = p0t.x * p1t.y - p0t.y * p1t.x;

    if ((e0 < 0.0f || e1 < 0.0f || e2 < 0.0f) && (e0 > 0.0f || e1 > 0.0f || e2 > 0.0f)) {
        return false;
    }

    float det = e0 + e1 + e2;
    if (FloatEqual(det, 0.0f)) {
        return false;
    }

    /* Value of parameter t & early rejections */
    float t = e0 * p0t.z + e1 * p1t.z + e2 * p2t.z;
    if (det > 0 && (t <= .0f || t >= r.max() * det)) {
        return false;
    }
    if (det < 0 && (t >= .0f || t <= r.max() * det)) {
        return false;
    }

    /* Barycentric coordinates */
    float idet = 1.f / det;
    float b0 = e0 * idet;
    float b1 = e1 * idet;
    float b2 = e2 * idet;
    t *= idet;

    /* Make sure t is larger than its error bound */
    float maxZt = MaxComponent(Abs(v3f{ p0t.z, p1t.z, p2t.z }));
    float deltaZ = gamma(3) * maxZt;

    float maxXt = MaxComponent(Abs(v3f{ p0t.x, p1t.x, p2t.x }));
    float deltaX = gamma(5) * (maxXt + maxZt);

    float maxYt = MaxComponent(Abs(v3f{ p0t.y, p1t.y, p2t.y }));
    float deltaY = gamma(5) * (maxYt + maxZt);

    float maxE = MaxComponent(Abs(v3f{ e0, e1, e2 }));
    float deltaE = 2.f * (gamma(2) * maxXt * maxYt + deltaY * maxXt + deltaX * maxYt);

    float deltaT = 3 * (gamma(3) * maxE * maxZt + deltaE * maxZt + deltaZ * maxE)
                   * std::abs(idet);
    if (t <= deltaT) {
        return false;
    }

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

    /* Compute dp/du & dp/dv */
    v3f dpdu, dpdv;
    v2f duv02 = uv0 - uv2;
    v2f duv12 = uv1 - uv2;

    float uvdet = duv02.x * duv12.y + duv12.x * duv02.y;
    if (!FloatEqual(uvdet, .0f)) {
        float invdet = 1.f / uvdet;
        dpdu = (duv12.y * dp02 - duv02.y * dp12) * invdet;
        dpdv = (-duv12.x * dp02 + duv02.x * dp12) * invdet;
    }
    else {
        CoordinateSystem(n, dpdu, dpdv);
    }

    /* Shading */
    v3f ns = n;
    if (vd->m_n) {
        ns = (b0 * vd->m_n[m_v[0]] + b1 * vd->m_n[m_v[1]] + b2 * vd->m_n[m_v[2]]);
    }

    /* Error */
    v3f err = { std::abs(b0 * p0.x) + std::abs(b1 * p1.x) + std::abs(b2 * p2.x),
                std::abs(b0 * p0.y) + std::abs(b1 * p1.y) + std::abs(b2 * p2.y),
                std::abs(b0 * p0.z) + std::abs(b1 * p1.z) + std::abs(b2 * p2.z) };

    /* Update Interaction */
    isect.t = t;
    isect.p = b0 * p0 + b1 * p1 + b2 * p2;
    isect.wo = -r.dir();
    isect.error = gamma(7) * err;
    isect.uv = b0 * uv0 + b1 * uv1 + b2 * uv2;
    isect.n = FaceForward(n, ns);
    isect.dpdu = dpdu;
    isect.dpdv = dpdv;

    /* Shading Geometry */
    isect.shading.n = ns;
    isect.shading.dpdu = dpdu;
    isect.shading.dpdv = dpdv;

    isect = m_md->m_objToWorld(isect);

    return true;
}

bool Triangle::qIntersect(const Ray &ray) const
{
    sptr<VertexData> vd = m_md->m_vd;

    /* Get triangle coordinates */
    v3f p0 = vd->m_v[m_v[0]];
    v3f p1 = vd->m_v[m_v[1]];
    v3f p2 = vd->m_v[m_v[2]];

    /* Transform to world space */
    Ray r = m_md->m_worldToObj(ray);

    /* Transform triangle into ray-space coordinates */
    v3f p0t = p0 - r.org();
    v3f p1t = p1 - r.org();
    v3f p2t = p2 - r.org();

    /* Permute */
    size_t kz = MaxDimension(Abs(r.dir()));
    size_t kx = (kz + 1) % 3;
    size_t ky = (kx + 1) % 3;

    v3f d = { r.dir()[kx], r.dir()[ky], r.dir()[kz] };

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
    auto e0 = p1t.x * p2t.y - p1t.y * p2t.x;
    auto e1 = p2t.x * p0t.y - p2t.y * p0t.x;
    auto e2 = p0t.x * p1t.y - p0t.y * p1t.x;

    if ((e0 < 0.0f || e1 < 0.0f || e2 < 0.0f) && (e0 > 0.0f || e1 > 0.0f || e2 > 0.0f)) {
        return false;
    }
    float det = e0 + e1 + e2;
    if (FloatEqual(det, 0.0f)) {
        return false;
    }
    float t = e0 * p0t.z + e1 * p1t.z + e2 * p2t.z;
    if (det > 0 && (t <= .0f || t >= r.max() * det)) {
        return false;
    }
    if (det < 0 && (t >= .0f || t <= r.max() * det)) {
        return false;
    }
    /* Barycentric coordinates */
    float idet = 1.f / det;
    t *= idet;

    /* Make sure t is larger than its error bound */
    float maxZt = MaxComponent(Abs(v3f{ p0t.z, p1t.z, p2t.z }));
    float deltaZ = gamma(3) * maxZt;

    float maxXt = MaxComponent(Abs(v3f{ p0t.x, p1t.x, p2t.x }));
    float deltaX = gamma(5) * (maxXt + maxZt);

    float maxYt = MaxComponent(Abs(v3f{ p0t.y, p1t.y, p2t.y }));
    float deltaY = gamma(5) * (maxYt + maxZt);

    float maxE = MaxComponent(Abs(v3f{ e0, e1, e2 }));
    float deltaE = 2.f * (gamma(2) * maxXt * maxYt + deltaY * maxXt + deltaX * maxYt);

    float deltaT = 3 * (gamma(3) * maxE * maxZt + deltaE * maxZt + deltaZ * maxE)
                   * std::abs(idet);
    return t > deltaT;
}

float Triangle::area() const
{
    const v3f p0 = Mulp(m_md->m_objToWorld, m_md->m_vd->m_v[m_v[0]]);
    const v3f p1 = Mulp(m_md->m_objToWorld, m_md->m_vd->m_v[m_v[1]]);
    const v3f p2 = Mulp(m_md->m_objToWorld, m_md->m_vd->m_v[m_v[2]]);

    return .5f * Cross(p1 - p0, p2 - p0).length();
}

bounds3f Triangle::bounds() const
{
    sptr<VertexData> vd = m_md->m_vd;

    const v3f &p0 = vd->m_v[m_v[0]];
    const v3f &p1 = vd->m_v[m_v[1]];
    const v3f &p2 = vd->m_v[m_v[2]];

    return m_md->m_objToWorld(Union(bounds3f(p0, p1), p2));
}

Interaction Triangle::sample(const v2f &u) const
{
    sptr<VertexData> vd = m_md->m_vd;

    v3f p0 = vd->m_v[m_v[0]];
    v3f p1 = vd->m_v[m_v[1]];
    v3f p2 = vd->m_v[m_v[2]];

    v2f b = UniformSampleTriangle(u);
    Interaction it;
    it.p = b.x * p0 + b.y * p1 + (1.0f - b.x - b.y) * p2;

    if (vd->m_n) {
        it.n = Normalize(b.x * vd->m_n[m_v[0]] + b.y * vd->m_n[m_v[1]]
                         + (1.0f - b.x - b.y) * vd->m_n[m_v[2]]);
    }
    else {
        it.n = Normalize(Cross(p1 - p0, p2 - p0));
    }
    it.p = Mulp(m_md->m_objToWorld, it.p);
    it.n = Muln(m_md->m_objToWorld, it.n);

    return it;
}

Interaction Triangle::sample(const Interaction &, const v2f &u) const
{
    return sample(u);
}

float Triangle::pdf(const Interaction &ref, const v3f &wi) const
{
    Ray r = SpawnRay(ref, wi);
    Interaction isect;
    if (!intersect(r, isect)) {
        return .0f;
    }
    return DistanceSquared(ref.p, isect.p) / (AbsDot(isect.n, -wi) * area());
}

#pragma mark - Mesh

struct _Mesh : Mesh {
    _Mesh(const sptr<MeshData> &md);

    bool intersect(const Ray &r, Interaction &isect) const override;
    bool qIntersect(const Ray &r) const override;

    float area() const override;
    bounds3f bounds() const override { return m_box; };
    Transform worldToObj() const override { return m_worldToObj; }

    Interaction sample(const v2f &u) const override;
    float pdf() const override;

    Interaction sample(const Interaction &ref, const v2f &u) const override;
    float pdf(const Interaction &ref, const v3f &wi) const override;

    std::vector<sptr<Shape>> faces() const override;

    bounds3f m_box;
    sptr<MeshData> m_md;
    std::vector<sptr<Triangle>> m_faces;
    Transform m_worldToObj;
};

_Mesh::_Mesh(const sptr<MeshData> &md)
{
    m_faces.reserve(md->m_np);
    for (size_t j = 0; j < md->m_np; j++) {
        m_faces.emplace_back(std::make_shared<Triangle>(md, j));
        m_box = Union(m_box, m_faces[j]->bounds());
    }
}

bool _Mesh::intersect(const Ray &r, Interaction &isect) const
{
    bool hit = false;
    float t = r.max();

    for (const auto &tri : m_faces) {
        if (tri->intersect({ r, t }, isect)) {
            t = isect.t;
            hit = true;
        }
    }
    return hit;
}

bool _Mesh::qIntersect(const Ray &r) const
{
    for (const auto &t : m_faces) {
        if (t->qIntersect(r)) {
            return true;
        }
    }
    return false;
}

float _Mesh::area() const
{
    /* This function should not be called, Triangle::area should be called instead */
    ASSERT(0);
    return .0f;
}

Interaction _Mesh::sample(const v2f &) const
{
    /* This function should not be called, Triangle::sample should be called instead */
    ASSERT(0);
    return {};
}

float _Mesh::pdf() const
{
    /* This function should not be called, Triangle::pdf should be called instead */
    ASSERT(0);
    return .0f;
}

Interaction _Mesh::sample(const Interaction &, const v2f &) const
{
    /* This function should not be called, Triangle::sample should be called instead */
    ASSERT(0);
    return {};
}

float _Mesh::pdf(const Interaction &, const v3f &) const
{
    /* This function should not be called, Triangle::pdf should be called instead */
    ASSERT(0);
    return .0f;
}

std::vector<sptr<Shape>> _Mesh::faces() const
{
    return std::vector<sptr<Shape>>(std::begin(m_faces), std::end(m_faces));
}

#pragma mark - Static Constructors

sptr<Mesh> Mesh::create(size_t nt,
                        const sptr<VertexData> &vd,
                        uptr<std::vector<uint32_t>> &i,
                        const Transform &worldToObj)
{
    sptr<MeshData> md = CreateMeshData(nt, vd, i, worldToObj);
    return std::make_shared<_Mesh>(md);
}
sptr<Mesh> Mesh::create(size_t nt,
                        uptr<std::vector<v3f>> &v,
                        uptr<std::vector<v3f>> &n,
                        uptr<std::vector<v2f>> &uv,
                        uptr<std::vector<uint32_t>> &i,
                        const Transform &worldToObj)
{
    sptr<VertexData> vd = VertexData::create(v->size(), v, n, uv);
    return Mesh::create(nt, vd, i, worldToObj);
}

sptr<Mesh> Mesh::create(const sptr<Params> &p)
{
    sptr<Value> vertices = Params::value(p, "vertices");
    sptr<Value> indices = Params::value(p, "indices");
    sptr<Value> count = Params::value(p, "count");

    if (count && vertices && indices) {
        sptr<Value> normals = Params::value(p, "normals");
        sptr<Value> texcoords = Params::value(p, "uv");
        Transform t = Transform(Params::matrix44f(p, "transform", m44f_identity()));

        size_t nt = count->u64();
        auto v = std::make_unique<std::vector<v3f>>(vertices->count());
        auto i = std::make_unique<std::vector<uint32_t>>(indices->count());

        vertices->value(TYPE_FLOAT32, v->data(), 0, 3 * vertices->count());
        indices->value(i->data(), 0, indices->count());

        uptr<std::vector<v3f>> n;
        if (normals) {
            n = std::make_unique<std::vector<v3f>>(normals->count());
            normals->value(TYPE_FLOAT32, n->data(), 0, 3 * normals->count());
        }
        uptr<std::vector<v2f>> uv;
        if (texcoords) {
            uv = std::make_unique<std::vector<v2f>>(texcoords->count());
            texcoords->value(TYPE_FLOAT32, uv->data(), 0, 2 * texcoords->count());
        }
        return Mesh::create(nt, v, n, uv, i, t);
    }
    ERROR_IF(!count, "Mesh parameter \"count\" not specified");
    ERROR_IF(!vertices, "Mesh parameter \"vertices\" not specified");
    ERROR_IF(!indices, "Mesh parameter \"indices\" not specified");

    return nullptr;
}
