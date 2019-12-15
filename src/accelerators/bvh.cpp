#include "accelerators/bvh.hpp"

#include "accelerators/bvhbuilder.hpp"

#include "rt1w/interaction.hpp"
#include "rt1w/ray.hpp"

#include <vector>

static bool box_hit(const bounds3f &b, const Ray &r)
{
    v3f dir = r.dir();
    v3f org = r.org();
    float tmax = r.max();
    float tmin = .0f;

    for (size_t i = 0; i < 3; i++) {
        float i_dir = 1.0f / (&dir.x)[i];
        float t0 = ((&b.lo.x)[i] - (&org.x)[i]) * i_dir;
        float t1 = ((&b.hi.x)[i] - (&org.x)[i]) * i_dir;
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

struct BVHLinearNode {
    bounds3f bounds;
    union {
        int32_t primitivesOffset;  /* For leaf nodes*/
        int32_t secondChildOffset; /* For interior nodes */
    };
    uint16_t size; /* Equals 0 for interior nodes */
    uint8_t axis;
    uint8_t pad[1]; /* Struct is 64bit for cache alignment*/
};

#pragma mark - BVH Accelerator

struct _BVHAccelerator : BVHAccelerator {
    _BVHAccelerator(const std::vector<sptr<Primitive>> &v) { init(v); }
    ~_BVHAccelerator() override { free(m_nodes); }

    bool intersect(const Ray &r, Interaction &isect) const override;
    bool qIntersect(const Ray &r) const override;
    bounds3f bounds() const override { return m_bounds; }
    sptr<AreaLight> light() const override;

    const std::vector<sptr<Primitive>> &primitives() const override { return m_prims; }

    void init(const std::vector<sptr<Primitive>> &prims);
    int32_t flattenBVH(const BVHBuildNode *root, int32_t &offset);

    std::vector<sptr<Primitive>> m_prims;
    bounds3f m_bounds;
    BVHLinearNode *m_nodes = nullptr;
};

sptr<AreaLight> _BVHAccelerator::light() const
{
    trap("BVHAccelerator::light() should never be called");
}

void _BVHAccelerator::init(const std::vector<sptr<Primitive>> &prims)
{
    auto builder = BVHBuilder(prims);

    /* Create structure for tree traversal */
    int32_t offset = 0;
    m_nodes = (BVHLinearNode *)malloc(builder.count() * sizeof(*m_nodes));
    flattenBVH(builder.root(), offset);

    m_prims = builder.prims();

    LOG("Created BVH with %lu nodes from %lu primitives",
        builder.count(),
        m_prims.size());
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

bool _BVHAccelerator::intersect(const Ray &r, Interaction &isect) const
{
    size_t index = 0;
    size_t next[64] = { 0 };
    size_t sp = 0;
    bool hit = false;
    float max = r.max();

    while (true) {
        if (box_hit(m_nodes[index].bounds, { r, max })) {
            size_t n = m_nodes[index].size;

            if (n > 0) {
                auto first = (size_t)m_nodes[index].primitivesOffset;

                for (size_t i = first; i < first + n; i++) {
                    if (m_prims[i]->intersect({ r, max }, isect)) {
                        hit = true;
                        max = isect.t;
                    }
                }
                if (sp == 0) {
                    break;
                }
                index = next[--sp];
            }
            else {
                v3f d = r.dir();
                if ((&d.x)[m_nodes[index].axis] < .0f) {
                    next[sp++] = index + 1;
                    index = (size_t)m_nodes[index].secondChildOffset;
                }
                else {
                    next[sp++] = (size_t)m_nodes[index].secondChildOffset;
                    index += 1;
                }
            }
        }
        else {
            if (sp == 0) {
                break;
            }
            index = next[--sp];
        }
    }
    return hit;
}

bool _BVHAccelerator::qIntersect(const Ray &r) const
{
    size_t index = 0;
    size_t next[64] = { 0 };
    size_t sp = 0;

    while (true) {
        if (box_hit(m_nodes[index].bounds, r)) {
            size_t n = m_nodes[index].size;

            if (n > 0) {
                auto first = (size_t)m_nodes[index].primitivesOffset;

                for (size_t i = first; i < first + n; ++i) {
                    if (m_prims[i]->qIntersect(r)) {
                        return true;
                    }
                }
                if (sp == 0) {
                    break;
                }
                index = next[--sp];
            }
            else {
                next[sp++] = (size_t)m_nodes[index].secondChildOffset;
                index += 1;
            }
        }
        else {
            if (sp == 0) {
                break;
            }
            index = next[--sp];
        }
    }
    return false;
}

sptr<BVHAccelerator> BVHAccelerator::create(const std::vector<sptr<Primitive>> &v)
{
    return std::make_shared<_BVHAccelerator>(v);
}
