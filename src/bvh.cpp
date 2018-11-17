#include "bvh.hpp"

#include "arena.hpp"

#include <vector>

static bool box_hit(const bounds3f &b, const sptr<Ray> &r, float tmin, float tmax)
{
    v3f dir = r->direction();
    v3f org = r->origin();
    float i_dir, t0, t1;

    for (size_t i = 0; i < 3; i++) {
        i_dir = 1.0f / (&dir.x)[i];
        t0 = ((&b.lo.x)[i] - (&org.x)[i]) * i_dir;
        t1 = ((&b.hi.x)[i] - (&org.x)[i]) * i_dir;
        if (i_dir < 0.0f) {
            std::swap(t0, t1);
        }
        tmin = t0 > tmin ? t0 : tmin;
        tmax = t1 < tmax ? t1 : tmax;
        if (tmax < tmin) {
            return false;
        }
    }
    return true;
}

#pragma mark - Utility structs

/* BVH Construction */
struct BVHPrimInfo {
    size_t   index;
    bounds3f bounds;
    v3f      center;
};

struct BVHBuildNode {

    void initLeaf(size_t ix, size_t n, const bounds3f &b) {
        children[0] = nullptr;
        children[1] = nullptr;
        index = ix;
        size = n;
        bounds = b;
    };

    void initInterior(int32_t ax, BVHBuildNode *c0, BVHBuildNode *c1) {
        children[0] = c0;
        children[1] = c1;
        axis = ax;
        bounds = Union(c0->bounds, c1->bounds);
        size = 0;
    };

    BVHBuildNode *children[2];
    bounds3f      bounds;
    int32_t       axis;
    size_t        index;
    size_t        size;
};

/* BVH traversal */
struct BVHLinearNode {
    bounds3f bounds;
    union {
        int32_t primitivesOffset;  /* For leaf nodes*/
        int32_t secondChildOffset; /* For interior nodes */
    };
    uint16_t size;                  /* Equals 0 for interior nodes */
    uint8_t  axis;
    uint8_t  pad[1];                /* Struct is 64bit for cache alignment*/
};

#pragma mark - BVH Accelerator

struct _BVHAccelerator : BVHAccelerator {

     _BVHAccelerator(const std::vector<sptr<Primitive>> &v) : m_prims(v) { buildBVH(); }
    ~_BVHAccelerator() override;

    bool     hit(const sptr<Ray> &r, float min, float max, hit_record &rec) const override;
    bounds3f bounds() const override { return m_bounds; }
    const std::vector<sptr<Primitive>> &primitives() const override { return m_prims; }

    void buildBVH();
    BVHBuildNode *buildNode(Arena *arena,
                            BVHPrimInfo *info,
                            size_t bgn,
                            size_t end,
                            size_t &node_count,
                            std::vector<sptr<Primitive>> &ordered);
    int32_t flattenBVH(const BVHBuildNode *root, int32_t &offset);

    std::vector<sptr<Primitive>> m_prims;
    bounds3f                     m_bounds;
    BVHLinearNode              * m_nodes = nullptr;
    size_t                       m_count = 0;
};

_BVHAccelerator::~_BVHAccelerator()
{
    free(m_nodes);
}

void _BVHAccelerator::buildBVH()
{
    uptr<Arena> arena = Arena::create();

    /* Get info for each primtive */
    BVHPrimInfo *info = (BVHPrimInfo *)arena->alloc(m_prims.size() * sizeof(*info));
    for (size_t i = 0; i < m_prims.size(); i++) {
        bounds3f b = m_prims[i]->bounds();
        info[i] = { i, b, b.center() };
    }

    /* Build tree structure */
    size_t count = 0;
    std::vector<sptr<Primitive>> ordered;
    BVHBuildNode *root = buildNode(arena.get(),info, 0, m_prims.size(), count, ordered);

    m_prims = std::move(ordered);

    /* Create structure for tree traversal */
    int32_t offset = 0;
    m_nodes = (BVHLinearNode *)malloc(count * sizeof(*m_nodes));
    flattenBVH(root, offset);
}

BVHBuildNode *_BVHAccelerator::buildNode(Arena *arena,
                                         BVHPrimInfo *const info,
                                         size_t bgn,
                                         size_t end,
                                         size_t &node_count,
                                         std::vector<sptr<Primitive>> &ordered)
{
    BVHBuildNode *node = (BVHBuildNode *)arena->alloc(sizeof(*node));
    node_count += 1;

    /* Calculate bounds for the primitives */
    bounds3f bounds;
    for (size_t i = bgn; i < end; i++) {
        bounds = Union(bounds, info[i].bounds);
    }

    /* Build a leaf node if there's only one primitive */
    size_t n = end - bgn;
    if (n == 1) {
        size_t ix = ordered.size();
        for (size_t i = bgn; i < end; i++) {
            ordered.push_back(m_prims[info[i].index]);
        }
        node->initLeaf(ix, n, bounds);

        return node;
    }

    bounds3f centerBounds;
    for (size_t i = bgn; i < end; i++) {
        centerBounds = Union(centerBounds, info[i].center);
    }
    int32_t axis = centerBounds.maxAxis();

    /* Partition the primitives into two subsets using the SAH heuristic */
    size_t mid;
    if (n <= 4) {
        mid = bgn + n / 2;
    }
    else {
        constexpr size_t nBuckets = 12;
        struct bucket {
            size_t   count = 0;
            bounds3f bounds;
        };
        struct bucket buckets[nBuckets];

        for (size_t i = bgn; i < end; i++) {
            size_t ix = (size_t)llrint(nBuckets *
                                       Offset(centerBounds, info[i].center)[(size_t)axis]);
            if (ix == nBuckets) {
                ix -= 1;
            }
            buckets[ix].count += 1;
            buckets[ix].bounds = Union(buckets[ix].bounds, info[i].bounds);
        }

        /* Calculate cost for split at each bucket */
        float costs[nBuckets - 1] = { 0.0 };
        for (size_t i = 0; i < nBuckets - 1; i++) {
            bounds3f b0, b1;
            size_t c0 = 0;
            size_t c1 = 0;

            for (size_t j = 0; j <= i ; j++) {
                b0 = Union(b0, buckets[j].bounds);
                c0 += buckets[j].count;
            }
            for (size_t j = i + 1; j < nBuckets; j++) {
                b1 = Union(b1, buckets[j].bounds);
                c1 += buckets[j].count;
            }
            costs[i] = .125f + (c0 * b0.area() + c1 * b1.area()) / bounds.area();
        }

        /* Find the minimum cost for the split */
        float minCost = costs[0];
        size_t minBucket = 0;
        for (size_t i = 1; i < nBuckets - 1; i++) {
            if (costs[i] < minCost) {
                minCost = costs[i];
                minBucket = i;
            }
        }

        float leafCost = n;
        if (minCost < leafCost) {
            auto part_fn = [=](const auto &p) {
                               size_t ix = (size_t)llrint(nBuckets *
                                                          Offset(centerBounds, p.center)[(size_t)axis]);
                               if (ix == nBuckets) {
                                   ix -= 1;
                               }
                               return ix <= minBucket;
                           };
            BVHPrimInfo *pmid = std::partition(&info[bgn], &info[end - 1] + 1, part_fn);
            mid = (size_t)(pmid - &info[0]);
        }
        else {
            size_t ix = ordered.size();
            for (size_t i = bgn; i < end; i++) {
                ordered.push_back(m_prims[info[i].index]);
            }
            node->initLeaf(ix, n, bounds);

            return node;
        }
    }

    /* Build Interior node with the two subsets */
    node->initInterior(axis,
                       buildNode(arena, info, bgn, mid, node_count, ordered),
                       buildNode(arena, info, mid, end, node_count, ordered));
    return node;
}

int32_t _BVHAccelerator::flattenBVH(const BVHBuildNode *root, int32_t &offset)
{
    ASSERT(root);

    BVHLinearNode &lnode = m_nodes[offset];
    int32_t savedOffset = offset++;

    lnode.bounds = root->bounds;
    if (root->size > 0) {
        lnode.primitivesOffset = (int32_t)root->index;
        lnode.size = (uint16_t)root->size;
    }
    else {
        lnode.axis = (uint8_t)root->axis;
        lnode.size = 0;
        flattenBVH(root->children[0], offset);
        lnode.secondChildOffset = flattenBVH(root->children[1], offset);
    }
    return savedOffset;
}

bool _BVHAccelerator::hit(const sptr<Ray> &r, float min, float max, hit_record &rec) const
{
    size_t index = 0;
    size_t next[64] = { 0 };
    size_t sp = 0;
    bool hit = false;

    while (true) {
        if (box_hit(m_nodes[index].bounds, r, min, max)) {
            size_t n = m_nodes[index].size;

            if (n > 0) {
                size_t first = (size_t)m_nodes[index].primitivesOffset;

                for (size_t i = first; i < first + n; i++) {
                    if (m_prims[i]->hit(r, min, max, rec)) {
                        hit = true;
                        max = rec.t;
                    }
                }
                if (sp == 0) { break; }
                index = next[--sp];
            }
            else {
                next[sp++] = (size_t)m_nodes[index].secondChildOffset;
                index += 1;
            }
        }
        else {
            if (sp == 0) { break; }
            index = next[--sp];
        }
    }
    return hit;
}

sptr<BVHAccelerator> BVHAccelerator::create(const std::vector<sptr<Primitive>> &v)
{
    return std::make_shared<_BVHAccelerator>(v);
}
