#pragma once

#include "rt1w/geometry.hpp"
#include "rt1w/shape.hpp"
#include "rt1w/sptr.hpp"

#include <vector>

struct MeshData;
struct Params;
struct Transform;
struct VertexData;

sptr<VertexData> CreateVertexData(size_t nv,
                                  uptr<std::vector<v3f>> &v,
                                  uptr<std::vector<v3f>> &n,
                                  uptr<std::vector<v2f>> &uv);
sptr<MeshData> CreateMeshData(size_t np,
                              const sptr<VertexData> &vd,
                              uptr<std::vector<uint32_t>> &i,
                              const Transform &worldToObj);

struct Mesh : Group {
    static sptr<Mesh> create(const sptr<Params> &p);
    static sptr<Mesh> create(size_t nt,
                             const sptr<VertexData> &vd,
                             uptr<std::vector<uint32_t>> &i,
                             const Transform &worldToObj);
    static sptr<Mesh> create(size_t nt,
                             uptr<std::vector<v3f>> &v,
                             uptr<std::vector<v3f>> &n,
                             uptr<std::vector<v2f>> &uv,
                             uptr<std::vector<uint32_t>> &i,
                             const Transform &worldToObj);
    virtual std::vector<sptr<Shape>> faces() const = 0;
};
