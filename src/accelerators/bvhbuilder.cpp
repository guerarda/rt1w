#include "bvhbuilder.hpp"

#include "rt1w/arena.hpp"
#include "rt1w/error.h"
#include "rt1w/primitive.hpp"

struct BVHPrimInfo {
    size_t index;
    bounds3f bounds;
    v3f center;
};

static BVHBuildNode *BuildNode(Arena *arena,
                               const std::vector<sptr<Primitive>> &prims,
                               BVHPrimInfo *info,
                               size_t bgn,
                               size_t end,
                               size_t &node_count,
                               std::vector<sptr<Primitive>> &ordered);

BVHBuilder::BVHBuilder(const std::vector<sptr<Primitive>> &prims)
{
    m_arena = Arena::create();

    /* Get info for each primtive */
    BVHPrimInfo *info = (BVHPrimInfo *)m_arena->alloc(prims.size() * sizeof(*info));
    for (size_t i = 0; i < prims.size(); ++i) {
        bounds3f b = prims[i]->bounds();
        info[i] = { i, b, b.center() };
    }

    /* Build tree structure */
    size_t count = 0;
    std::vector<sptr<Primitive>> ordered;
    m_root = BuildNode(m_arena.get(), prims, info, 0, prims.size(), count, ordered);
    m_count = count;
    m_prims = std::move(ordered);
}

BVHBuildNode *BuildNode(Arena *arena,
                        const std::vector<sptr<Primitive>> &prims,
                        BVHPrimInfo *info,
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
            ordered.push_back(prims[info[i].index]);
        }
        node->initLeaf(ix, n, bounds);

        return node;
    }

    bounds3f centerBounds;
    for (size_t i = bgn; i < end; i++) {
        centerBounds = Union(centerBounds, info[i].center);
    }
    auto axis = (size_t)centerBounds.maxAxis();

    /* Partition the primitives into two subsets using the SAH heuristic */
    size_t mid;
    if (n <= 4) {
        mid = bgn + n / 2;
    }
    else {
        constexpr size_t nBuckets = 12;
        struct bucket {
            size_t count = 0;
            bounds3f bounds;
        };
        struct bucket buckets[nBuckets];

        for (size_t i = bgn; i < end; i++) {
            size_t ix = (size_t)llrint(nBuckets
                                       * Offset(centerBounds, info[i].center)[axis]);
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

            for (size_t j = 0; j <= i; j++) {
                b0 = Union(b0, buckets[j].bounds);
                c0 += buckets[j].count;
            }
            for (size_t j = i + 1; j < nBuckets; j++) {
                b1 = Union(b1, buckets[j].bounds);
                c1 += buckets[j].count;
            }
            costs[i] = 1.f + (c0 * b0.area() + c1 * b1.area()) / bounds.area();
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
                auto ix = (size_t)llrint(nBuckets * Offset(centerBounds, p.center)[axis]);
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
                ordered.push_back(prims[info[i].index]);
            }
            node->initLeaf(ix, n, bounds);

            return node;
        }
    }

    /* Build Interior node with the two subsets */
    node->initInterior((int32_t)axis,
                       BuildNode(arena, prims, info, bgn, mid, node_count, ordered),
                       BuildNode(arena, prims, info, mid, end, node_count, ordered));
    return node;
}
