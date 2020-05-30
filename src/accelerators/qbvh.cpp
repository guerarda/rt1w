#include "qbvh.hpp"

#include "bvhbuilder.hpp"

#include "rt1w/error.h"
#include "rt1w/geometry.hpp"
#include "rt1w/interaction.hpp"
#include "rt1w/ray.hpp"

#include <queue>
#include <vector>

#include <smmintrin.h>
#include <x86intrin.h>
#include <xmmintrin.h>

static uint32_t box_hit(const float *b, const Ray &r)
{
    v3f dir = r.dir();
    v3f org = r.org();

    __m128 tmin = _mm_setzero_ps();
    __m128 tmax = _mm_set1_ps(r.max());

    for (size_t i = 0; i < 3; ++i) {
        __m128 d = _mm_set1_ps(1.f / (&dir.x)[i]);
        __m128 o = _mm_set1_ps((&org.x)[i]);

        __m128 lo = _mm_load_ps(&b[4 * i]);
        __m128 hi = _mm_load_ps(&b[4 * i + 12]);

        __m128 t0 = _mm_mul_ps(d, _mm_sub_ps(lo, o));
        __m128 t1 = _mm_mul_ps(d, _mm_sub_ps(hi, o));

        __m128 mask = _mm_cmplt_ps(d, _mm_setzero_ps());

        __m128 near = _mm_blendv_ps(t0, t1, mask);
        __m128 far = _mm_blendv_ps(t1, t0, mask);

        tmin = _mm_blendv_ps(tmin, near, _mm_cmplt_ps(tmin, near));
        tmax = _mm_blendv_ps(tmax, far, _mm_cmpgt_ps(tmax, far));
    }
    int32_t mask = _mm_movemask_ps(_mm_cmple_ps(tmin, tmax));

    return (mask & 0x1 ? 0xFF : 0x0) | (mask & 0x2 ? 0xFF00 : 0x0)
           | (mask & 0x4 ? 0xFF000 : 0x0) | (mask & 0x8 ? 0xFF000000 : 0x0);
}

struct QBVHNode {
    float bounds[24]; /* |b0-3.lo.x|b0-3.lo.y|b0-3.lo.z|b0-3.hi.x|b0-3.hi.y|b0-3.hi.z| */
    int32_t child[4]; /* Index of the 4 children */
    int32_t axis[3];  /* Axis used for splits */
    int32_t pad[1];   /* Pad struct to be 128 bytes */
};

static size_t leafPrimitiveIndex(int32_t ix)
{
    return ix & 0x07FFFFFF;
}

static size_t leafPrimitiveCount(int32_t ix)
{
    return (ix & 0x78000000) >> 27;
}

static int32_t leafIndex(size_t ix, size_t count)
{
    ASSERT(ix < 0x08000000);
    ASSERT(count < 16);

    return (int32_t)(0x80000000 | (count << 27) | (ix & 0x07FFFFFF));
}

static void initBounds(const bounds3f *a, float *b)
{
    for (size_t i = 0; i < 4; ++i) {
        b[i] = a[i].lo.x;
        b[4 + i] = a[i].lo.y;
        b[8 + i] = a[i].lo.z;
        b[12 + i] = a[i].hi.x;
        b[16 + i] = a[i].hi.y;
        b[20 + i] = a[i].hi.z;
    }
}

#pragma mark - BVH Accelerator

struct _QBVHAccelerator : QBVHAccelerator {
    _QBVHAccelerator(const std::vector<sptr<Primitive>> &v) { init(v); }

    bool intersect(const Ray &r, Interaction &isect) const override;
    bool qIntersect(const Ray &r) const override;
    bounds3f bounds() const override { return m_bounds; }
    sptr<AreaLight> light() const override;

    const std::vector<sptr<Primitive>> &primitives() const override { return m_prims; }

    void init(const std::vector<sptr<Primitive>> &prims);
    void flattenBVH(const BVHBuildNode *root);

    std::vector<sptr<Primitive>> m_prims;
    bounds3f m_bounds;
    std::vector<QBVHNode> m_nodes;
};

void _QBVHAccelerator::init(const std::vector<sptr<Primitive>> &prims)
{
    auto builder = BVHBuilder(prims);
    flattenBVH(builder.root());

    m_prims = builder.prims();

    LOG("Created QBVH with %lu nodes from %lu primitives",
        m_nodes.size(),
        m_prims.size());
}

sptr<AreaLight> _QBVHAccelerator::light() const
{
    trap("QBVHAccelerator::light() should never be called");
}

void _QBVHAccelerator::flattenBVH(const BVHBuildNode *root)
{
    ASSERT(root);

    if (root->size > 0) {
        QBVHNode node;
        bounds3f bounds[4] = {};

        node.child[0] = leafIndex(root->index, root->size);
        initBounds(bounds, node.bounds);

        m_nodes.push_back(node);
        return;
    }

    int32_t ix = 0;
    std::queue<const BVHBuildNode *> queue;

    queue.push(root);
    while (!queue.empty()) {
        const auto *bnode = queue.front();
        queue.pop();

        bounds3f bounds[4] = {};
        BVHBuildNode *children[4] = {};

        QBVHNode qnode;
        qnode.axis[0] = bnode->axis;

        for (size_t i = 0; i < 2; ++i) {
            auto *bchild = bnode->children[i];
            if (bchild->size > 0) {
                children[2 * i] = bchild;
            }
            else {
                qnode.axis[i + 1] = bchild->axis;
                children[2 * i] = bchild->children[0];
                children[2 * i + 1] = bchild->children[1];
            }
        }

        for (size_t i = 0; i < 4; ++i) {
            if (auto *child = children[i]) {
                bounds[i] = child->bounds;

                if (child->size > 0) {
                    qnode.child[i] = leafIndex(child->index, child->size);
                }
                else {
                    queue.push(child);
                    qnode.child[i] = ++ix;
                }
            }
        }
        initBounds(bounds, qnode.bounds);
        m_nodes.emplace_back(qnode);
    }
}

constexpr size_t kStackSize = 64;

bool _QBVHAccelerator::intersect(const Ray &r, Interaction &isect) const
{
    bool hit = false;
    float max = r.max();

    int32_t index = 0;
    int32_t next[kStackSize] = { 0 };
    size_t sp = 0;

    while (true) {
        if (index < 0) {
            /* Leaf node  */
            auto first = leafPrimitiveIndex(index);
            auto n = leafPrimitiveCount(index);

            for (size_t i = first; i < first + n; ++i) {
                if (m_prims[i]->intersect({ r, max }, isect)) {
                    hit = true;
                    max = isect.t;
                }
            }
        }
        else {
            /* Interior Node  */
            auto ix = (size_t)index;
            const auto &node = m_nodes[ix];

            auto result = box_hit(node.bounds, { r, max });
            if (result) {
                int32_t indexes[4];
                memcpy(indexes, node.child, sizeof(indexes));

                /* Re-arrange nodes to visit according to ray direction */
                v3f d = r.dir();
                if ((&d.x)[(size_t)node.axis[0]] < .0f) {
                    std::swap(indexes[0], indexes[2]);
                    std::swap(indexes[1], indexes[3]);
                    result = (result & 0x0000FFFF) << 16 | (result & 0xFFFF0000) >> 16;
                }
                if ((&d.x)[(size_t)node.axis[1]] < .0f) {
                    std::swap(indexes[0], indexes[1]);
                    result = ((result & 0x00FF0000) << 8 | (result & 0xFF000000) >> 8)
                             | (result & 0x0000FFFF);
                }
                if ((&d.x)[(size_t)node.axis[2]] < .0f) {
                    std::swap(indexes[2], indexes[3]);
                    result = (result & 0xFFFF0000)
                             | ((result & 0x000000FF) << 8 | (result & 0x0000FF00) >> 8);
                }
                for (size_t i = 0; i < 4; ++i) {
                    if (result & (0xFFu << 8 * i)) {
                        ASSERT(sp < kStackSize);
                        next[sp++] = indexes[i];
                    }
                }
            }
        }
        if (sp == 0) {
            break;
        }
        index = next[--sp];
    }
    return hit;
}

bool _QBVHAccelerator::qIntersect(const Ray &r) const
{
    int32_t index = 0;
    int32_t next[kStackSize] = { 0 };
    size_t sp = 0;

    while (true) {
        if (index < 0) {
            /* Leaf node  */
            auto first = leafPrimitiveIndex(index);
            auto n = leafPrimitiveCount(index);

            for (size_t i = first; i < first + n; ++i) {
                if (m_prims[i]->qIntersect(r)) {
                    return true;
                }
            }
        }
        else {
            /* Interior Node  */
            auto ix = (size_t)index;
            const auto &node = m_nodes[ix];

            auto result = box_hit(node.bounds, r);
            if (result) {
                int32_t indexes[4];
                memcpy(indexes, node.child, sizeof(indexes));

                for (size_t i = 0; i < 4; ++i) {
                    if (result & (0xFFu << 8 * i)) {
                        ASSERT(sp < kStackSize);
                        next[sp++] = indexes[i];
                    }
                }
            }
        }
        if (sp == 0) {
            break;
        }
        index = next[--sp];
    }
    return false;
}

sptr<QBVHAccelerator> QBVHAccelerator::create(const std::vector<sptr<Primitive>> &v)
{
    return std::make_shared<_QBVHAccelerator>(v);
}
