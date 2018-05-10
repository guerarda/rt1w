#include "params.hpp"

#include "texture.hpp"

struct _Params : Params {

    void insert(const std::string &k, const sptr<Params> &v);
    void insert(const std::string &k, const std::string &v);
    void insert(const std::string &k, const sptr<Texture> &v);
    void insert(const std::string &k, const sptr<Value> &v);

    void merge(const sptr<Params> &p);
    void merge(const std::map<const std::string, const sptr<Params>> &params);
    void merge(const std::map<const std::string, const std::string> &strings);
    void merge(const std::map<const std::string, const sptr<Texture>> &textures);
    void merge(const std::map<const std::string, const sptr<Value>> &values);

    sptr<Params>  params(const std::string &k) const;
    std::string   string(const std::string &j) const;
    sptr<Texture> texture(const std::string &k) const;
    sptr<Value>   value(const std::string &k) const;

    std::map<const std::string, const sptr<Params>>  m_params;
    std::map<const std::string, const std::string>   m_strings;
    std::map<const std::string, const sptr<Texture>> m_textures;
    std::map<const std::string, const sptr<Value>>   m_values;
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
    return nullptr;
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
