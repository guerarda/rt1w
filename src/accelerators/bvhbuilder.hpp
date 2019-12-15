#pragma once

#include "rt1w/arena.hpp"
#include "rt1w/geometry.hpp"
#include "rt1w/sptr.hpp"

#include <vector>

struct Primitive;

struct BVHBuildNode {
    void initLeaf(size_t ix, size_t n, const bounds3f &b)
    {
        bounds = b;
        children[0] = children[1] = nullptr;
        axis = -1;
        index = ix;
        size = n;
    };

    void initInterior(int32_t a, BVHBuildNode *c0, BVHBuildNode *c1)
    {
        bounds = Union(c0->bounds, c1->bounds);
        children[0] = c0;
        children[1] = c1;
        axis = a;
        index = 0;
        size = 0;
    };

    bounds3f bounds;
    BVHBuildNode *children[2];
    int32_t axis;
    size_t index;
    size_t size;
};

struct BVHBuilder {
    BVHBuilder(const std::vector<sptr<Primitive>> &prims);
    BVHBuilder() = delete;

    BVHBuildNode *root() const { return m_root; }
    size_t count() const { return m_count; }
    const std::vector<sptr<Primitive>> &prims() const { return m_prims; }

private:
    BVHBuildNode *m_root;
    size_t m_count;
    std::vector<sptr<Primitive>> m_prims;
    uptr<Arena> m_arena;
};
