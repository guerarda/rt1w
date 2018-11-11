#include "light.hpp"

#include "error.h"
#include "texture.hpp"
#include "params.hpp"

struct _DiffuseLight : DiffuseLight {

    _DiffuseLight(const sptr<Texture> &emit) : m_emit(emit) { }

    bool scatter(const sptr<Ray> &,
                 const hit_record &,
                 v3f &,
                 v3f &) const   { return false; }
    v3f emitted(float u, float v, v3f p) const { return m_emit->value(u, v, p); }

    sptr<Texture> m_emit;
};

#pragma mark - Static Constructors

sptr<DiffuseLight> DiffuseLight::create(const sptr<Texture> &emit)
{
    return std::make_shared<_DiffuseLight>(emit);
}

sptr<DiffuseLight> DiffuseLight::create(const sptr<Params> &p)
{
    sptr<Texture> emit = p->texture("emit");

    if (emit) {
        return DiffuseLight::create(emit);
    }
    warning("Diffuse Light parameter \"emit\" not specified");
    return nullptr;
}
