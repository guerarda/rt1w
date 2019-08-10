#pragma once

#include "shapes/mesh.hpp"

#include "rt1w/transform.hpp"

#pragma mark - Vertex Data

struct VertexData : Object {
    static sptr<VertexData> create(size_t nv,
                                   uptr<std::vector<v3f>> &v,
                                   uptr<std::vector<v3f>> &n,
                                   uptr<std::vector<v2f>> &uv)
    {
        return std::make_shared<VertexData>(nv, v, n, uv);
    }

    VertexData(size_t nv,
               uptr<std::vector<v3f>> &v,
               uptr<std::vector<v3f>> &n,
               uptr<std::vector<v2f>> &uv) :
        m_nv(nv),
        m_v(v ? v->data() : nullptr),
        m_n(n ? n->data() : nullptr),
        m_uv(uv ? uv->data() : nullptr),
        m_data({ std::move(v), std::move(n), std::move(uv) })
    {}

    const size_t m_nv;
    const v3f *m_v;
    const v3f *m_n;
    const v2f *m_uv;

    struct {
        uptr<const std::vector<v3f>> v;
        uptr<const std::vector<v3f>> n;
        uptr<const std::vector<v2f>> uv;
    } m_data;
};

sptr<VertexData> CreateVertexData(size_t nv,
                                  uptr<std::vector<v3f>> &v,
                                  uptr<std::vector<v3f>> &n,
                                  uptr<std::vector<v2f>> &uv)
{
    return VertexData::create(nv, v, n, uv);
}

#pragma mark - Mesh Data

struct MeshData : Object {
    static sptr<MeshData> create(size_t np,
                                 const sptr<VertexData> &vd,
                                 uptr<std::vector<uint32_t>> &i,
                                 const Transform &worldToObj)
    {
        return std::make_shared<MeshData>(np, vd, i, worldToObj);
    }

    MeshData(size_t np,
             const sptr<VertexData> &vd,
             uptr<std::vector<uint32_t>> &i,
             const Transform &worldToObj) :
        m_np(np),
        m_ni(i->size()),
        m_i(i->data()),
        m_vd(vd),
        m_worldToObj(worldToObj),
        m_objToWorld(Inverse(worldToObj)),
        m_data({ std::move(i) })
    {}

    const size_t m_np;
    const size_t m_ni;
    const uint32_t *m_i;
    const sptr<VertexData> m_vd;
    const Transform m_worldToObj;
    const Transform m_objToWorld;
    struct {
        uptr<std::vector<uint32_t>> i;
    } m_data;
};

sptr<MeshData> CreateMeshData(size_t np,
                              const sptr<VertexData> &vd,
                              uptr<std::vector<uint32_t>> &i,
                              const Transform &worldToObj)
{
    return MeshData::create(np, vd, i, worldToObj);
}
