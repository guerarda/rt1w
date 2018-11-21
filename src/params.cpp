#include "params.hpp"

#include "texture.hpp"
#include "value.hpp"

struct _Params : Params {
    void insert(const std::string &k, const sptr<Params> &v) override;
    void insert(const std::string &k, const std::string &v) override;
    void insert(const std::string &k, const sptr<Texture> &v) override;
    void insert(const std::string &k, const sptr<Value> &v) override;

    void merge(const sptr<Params> &p) override;
    void merge(const std::map<const std::string, const sptr<Params>> &params) override;
    void merge(const std::map<const std::string, const std::string> &strings) override;
    void merge(const std::map<const std::string, const sptr<Texture>> &textures) override;
    void merge(const std::map<const std::string, const sptr<Value>> &values) override;

    sptr<Params> params(const std::string &k) const override;
    std::string string(const std::string &k) const override;
    sptr<Texture> texture(const std::string &k) const override;
    sptr<Value> value(const std::string &k) const override;

    std::map<const std::string, const sptr<Params>> m_params;
    std::map<const std::string, const std::string> m_strings;
    std::map<const std::string, const sptr<Texture>> m_textures;
    std::map<const std::string, const sptr<Value>> m_values;
};

void _Params::insert(const std::string &k, const sptr<Params> &v)
{
    m_params.insert(std::make_pair(k, v));
}

void _Params::insert(const std::string &k, const std::string &v)
{
    m_strings.insert(std::make_pair(k, v));
}

void _Params::insert(const std::string &k, const sptr<Texture> &v)
{
    m_textures.insert(std::make_pair(k, v));
}

void _Params::insert(const std::string &k, const sptr<Value> &v)
{
    m_values.insert(std::make_pair(k, v));
}

void _Params::merge(const sptr<Params> &p)
{
    if (p) {
        if (sptr<_Params> other = std::static_pointer_cast<_Params>(p)) {
            auto params = other->m_params;
            auto strings = other->m_strings;
            auto textures = other->m_textures;
            auto values = other->m_values;

            params.insert(m_params.begin(), m_params.end());
            strings.insert(m_strings.begin(), m_strings.end());
            textures.insert(m_textures.begin(), m_textures.end());
            values.insert(m_values.begin(), m_values.end());

            m_params = std::move(params);
            m_strings = std::move(strings);
            m_textures = std::move(textures);
            m_values = std::move(values);
        }
    }
}

void _Params::merge(const std::map<const std::string, const sptr<Params>> &params)
{
    auto map = params;
    map.insert(m_params.begin(), m_params.end());
    m_params = std::move(map);
}

void _Params::merge(const std::map<const std::string, const std::string> &strings)
{
    auto map = strings;
    map.insert(m_strings.begin(), m_strings.end());
    m_strings = std::move(map);
}

void _Params::merge(const std::map<const std::string, const sptr<Texture>> &textures)
{
    auto map = textures;
    map.insert(m_textures.begin(), m_textures.end());
    m_textures = std::move(map);
}

void _Params::merge(const std::map<const std::string, const sptr<Value>> &values)
{
    auto map = values;
    map.insert(m_values.begin(), m_values.end());
    m_values = std::move(map);
}

sptr<Params> _Params::params(const std::string &k) const
{
    auto it = m_params.find(k);
    if (it != m_params.end()) {
        return it->second;
    }
    return nullptr;
}

std::string _Params::string(const std::string &k) const
{
    auto it = m_strings.find(k);
    if (it != m_strings.end()) {
        return it->second;
    }
    return std::string();
}

sptr<Texture> _Params::texture(const std::string &k) const
{
    auto itt = m_textures.find(k);
    if (itt != m_textures.end()) {
        return itt->second;
    }

    auto itp = m_params.find(k);
    if (itp != m_params.end()) {
        return Texture::create(itp->second);
    }

    auto its = m_strings.find(k);
    if (its != m_strings.end()) {
        return texture(its->second);
    }
    return nullptr;
}

sptr<Value> _Params::value(const std::string &k) const
{
    auto itv = m_values.find(k);
    if (itv != m_values.end()) {
        return itv->second;
    }
    auto its = m_strings.find(k);
    if (its != m_strings.end()) {
        return value(its->second);
    }
    return nullptr;
}

#pragma mark - Static constructors

sptr<Params> Params::create()
{
    return std::make_shared<_Params>();
}

#pragma mark - Static functions

int32_t Params::i32(const sptr<Params> &p, const std::string &n, int32_t v)
{
    ASSERT(p);
    if (auto val = p->value(n)) {
        return val->i32();
    }
    return v;
}

int64_t Params::i64(const sptr<Params> &p, const std::string &n, int64_t v)
{
    ASSERT(p);
    if (auto val = p->value(n)) {
        return val->i64();
    }
    return v;
}

uint32_t Params::u32(const sptr<Params> &p, const std::string &n, uint32_t v)
{
    ASSERT(p);
    if (auto val = p->value(n)) {
        return val->u32();
    }
    return v;
}

uint64_t Params::u64(const sptr<Params> &p, const std::string &n, uint64_t v)
{
    ASSERT(p);
    if (auto val = p->value(n)) {
        return val->u64();
    }
    return v;
}

float Params::f32(const sptr<Params> &p, const std::string &n, float v)
{
    ASSERT(p);
    if (auto val = p->value(n)) {
        return val->f32();
    }
    return v;
}

double Params::f64(const sptr<Params> &p, const std::string &n, double v)
{
    ASSERT(p);
    if (auto val = p->value(n)) {
        return val->f64();
    }
    return v;
}

v2i Params::vector2i(const sptr<Params> &p, const std::string &n, v2i v)
{
    ASSERT(p);
    if (auto val = p->value(n)) {
        return val->vector2i();
    }
    return v;
}

v2u Params::vector2u(const sptr<Params> &p, const std::string &n, v2u v)
{
    ASSERT(p);
    if (auto val = p->value(n)) {
        return val->vector2u();
    }
    return v;
}

v3f Params::vector3f(const sptr<Params> &p, const std::string &n, v3f v)
{
    ASSERT(p);
    if (auto val = p->value(n)) {
        return val->vector3f();
    }
    return v;
}

v3d Params::vector3d(const sptr<Params> &p, const std::string &n, v3d v)
{
    ASSERT(p);
    if (auto val = p->value(n)) {
        return val->vector3d();
    }
    return v;
}

std::string Params::string(const sptr<Params> &p,
                           const std::string &n,
                           const std::string &v)
{
    ASSERT(p);
    auto str = p->string(n);
    return str.empty() ? v : str;
}

m44f Params::matrix4x4f(const sptr<Params> &p, const std::string &n, m44f v)
{
    ASSERT(p);
    if (auto val = p->value(n)) {
        m44f m;
        val->value(TYPE_FLOAT32, &m.vx.x, 0, 16);

        return m;
    }
    return v;
}
