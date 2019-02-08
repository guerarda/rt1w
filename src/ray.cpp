#include "ray.hpp"

#include "geometry.hpp"
#include "interaction.hpp"
#include "utils.hpp"

static inline v3f OffsetRayOrigin(const v3f &p,
                                  const v3f &error,
                                  const v3f &n,
                                  const v3f &w)
{
    float d = Dot(error, Abs(n));
    v3f offset = d * n;
    if (Dot(offset, w) < .0f) {
        offset = -offset;
    }
    v3f org = p + offset;
    org.x = offset.x > .0f ? NextFloatUp(org.x) : NextFloatDown(org.x);
    org.y = offset.y > .0f ? NextFloatUp(org.y) : NextFloatDown(org.y);
    org.z = offset.z > .0f ? NextFloatUp(org.z) : NextFloatDown(org.z);

    return org;
}

Ray SpawnRay(const Interaction &i, v3f dir)
{
    ASSERT(!HasNaN(i));
    ASSERT(!HasNaN(dir));

    v3f org = OffsetRayOrigin(i.p, i.error, i.n, dir);
    return { org, dir };
}

Ray SpawnRayTo(const Interaction &i, v3f p)
{
    ASSERT(!HasNaN(i));
    ASSERT(!HasNaN(p));

    v3f org = OffsetRayOrigin(i.p, i.error, i.n, p - i.p);
    v3f dir = p - org;
    return { org, dir };
}

Ray SpawnRayTo(const Interaction &i, const Interaction &t)
{
    ASSERT(!HasNaN(i));
    ASSERT(!HasNaN(t));

    v3f org = OffsetRayOrigin(i.p, i.error, i.n, t.p - i.p);
    v3f dst = OffsetRayOrigin(t.p, t.error, t.n, org - t.p);
    v3f dir = dst - org;
    return { org, dir };
}
