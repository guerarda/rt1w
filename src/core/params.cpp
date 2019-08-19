#include "rt1w/params.hpp"

#include "rt1w/texture.hpp"
#include "rt1w/value.hpp"

// clang-format off
template <typename T> struct ptype { typedef T type; };
template <> struct ptype<std::string> { typedef std::string type; };
template <> struct ptype<Params> { typedef sptr<Params> type; };
template <> struct ptype<Shape> { typedef sptr<Shape> type; };
template <> struct ptype<Texture> { typedef sptr<Texture> type; };
template <> struct ptype<Value> { typedef sptr<Value> type; };

struct _Params : Params {
    void insert(const std::string &k, const sptr<Params> &v) override { insert<Params>(k, v); }
    void insert(const std::string &k, const sptr<Shape> &v) override { insert<Shape>(k, v); }
    void insert(const std::string &k, const std::string &v) override { insert<std::string>(k, v); }
    void insert(const std::string &k, const sptr<Texture> &v) override { insert<Texture>(k, v); }
    void insert(const std::string &k, const sptr<Value> &v) override { insert<Value>(k, v); }

    void merge(const sptr<Params> &p) override;
    void merge(const std::map<std::string, sptr<Params>> &m) override { merge<Params>(m); }
    void merge(const std::map<std::string, sptr<Shape>> &m) override { merge<Shape>(m); }
    void merge(const std::map<std::string, std::string> &m) override  { merge<std::string>(m); }
    void merge(const std::map<std::string, sptr<Texture>> &m) override { merge<Texture>(m); }
    void merge(const std::map<std::string, sptr<Value>> &m) override  { merge<Value>(m); }

    sptr<Params> params(const std::string &k) const override { return find_recursive<Params>(k); }
    sptr<Shape> shape(const std::string &k) const override { return find_recursive<Shape>(k); }
    std::string string(const std::string &k) const override  { return find<std::string>(k); }
    sptr<Texture> texture(const std::string &k) const override { return find_create<Texture>(k); }
    sptr<Value> value(const std::string &k) const override { return find_recursive<Value>(k); }

    template <typename T> std::map<std::string, typename ptype<T>::type> &pmap();
    template <typename T> const std::map<std::string, typename ptype<T>::type> &const_pmap() const;
    // clang-format on

    template <typename T>
    void insert(const std::string &k, const typename ptype<T>::type &v)
    {
        pmap<T>()[k] = v;
    }

    template <typename T>
    void merge(const std::map<std::string, typename ptype<T>::type> &v)
    {
        auto map = v;
        map.insert(std::begin(pmap<T>()), std::end(pmap<T>()));
        pmap<T>() = std::move(map);
    }

    template <typename T>
    typename ptype<T>::type find(const std::string &k) const
    {
        auto map = const_pmap<T>();
        auto it = map.find(k);
        if (it != std::end(map)) {
            return it->second;
        }
        return {};
    }

    template <typename T>
    typename ptype<T>::type find_recursive(const std::string &k) const
    {
        if (auto v = find<T>(k)) {
            return v;
        }
        auto it = m_strings.find(k);
        if (it != std::end(m_strings)) {
            return find_recursive<T>(it->second);
        }
        return {};
    }

    template <typename T>
    typename ptype<T>::type find_create(const std::string &k) const
    {
        if (auto v = find<T>(k)) {
            return v;
        }
        auto itp = m_params.find(k);
        if (itp != std::end(m_params)) {
            return T::create(itp->second);
        }
        auto it = m_strings.find(k);
        if (it != std::end(m_strings)) {
            return find_create<T>(it->second);
        }
        return {};
    }

    std::map<std::string, sptr<Params>> m_params;
    std::map<std::string, sptr<Shape>> m_shapes;
    std::map<std::string, std::string> m_strings;
    std::map<std::string, sptr<Texture>> m_textures;
    std::map<std::string, sptr<Value>> m_values;
};

void _Params::merge(const sptr<Params> &p)
{
    if (p) {
        if (sptr<_Params> other = std::static_pointer_cast<_Params>(p)) {
            merge(other->m_params);
            merge(other->m_strings);
            merge(other->m_shapes);
            merge(other->m_textures);
            merge(other->m_values);
        }
    }
}

#pragma mark - Static constructors

sptr<Params> Params::create()
{
    return std::make_shared<_Params>();
}

#pragma mark - Static Getter Functions

template <typename T>
T Params::scalarp(const sptr<const Params> &p, const std::string &k, T v)
{
    ASSERT(p);
    if (auto value = p->value(k)) {
        T rv;
        value->value(&rv, 0, 1);
        return rv;
    }
    return v;
}
template <typename T, typename U>
U Params::vectorp(const sptr<const Params> &p, const std::string &k, U v)
{
    ASSERT(p);
    if (auto value = p->value(k)) {
        U rv;
        value->value((T *)&rv, 0, sizeof(U) / sizeof(T));
        return rv;
    }
    return v;
}

std::string Params::string(const sptr<const Params> &p,
                           const std::string &k,
                           const std::string &v)
{
    ASSERT(p);
    auto str = p->string(k);
    return str.empty() ? v : str;
}

#pragma mark - Explicit Template Instantiation
// clang-format off
template <> std::map<std::string, ptype<std::string>::type> &_Params::pmap<std::string>() { return m_strings; }
template <> std::map<std::string, ptype<Params>::type>      &_Params::pmap<Params>()      { return m_params; }
template <> std::map<std::string, ptype<Shape>::type>       &_Params::pmap<Shape>()       { return m_shapes; }
template <> std::map<std::string, ptype<Texture>::type>     &_Params::pmap<Texture>()     { return m_textures; }
template <> std::map<std::string, ptype<Value>::type>       &_Params::pmap<Value>()       { return m_values; }

template <> const std::map<std::string, ptype<std::string>::type> &_Params::const_pmap<std::string>() const { return m_strings; }
template <> const std::map<std::string, ptype<Params>::type>      &_Params::const_pmap<Params>() const      { return m_params; }
template <> const std::map<std::string, ptype<Shape>::type>       &_Params::const_pmap<Shape>() const       { return m_shapes; }
template <> const std::map<std::string, ptype<Texture>::type>     &_Params::const_pmap<Texture>() const     { return m_textures; }
template <> const std::map<std::string, ptype<Value>::type>       &_Params::const_pmap<Value>() const       { return m_values; }

template int32_t  Params::scalarp(const sptr<const Params> &p, const std::string &k, int32_t v);
template int64_t  Params::scalarp(const sptr<const Params> &p, const std::string &k, int64_t v);
template uint32_t Params::scalarp(const sptr<const Params> &p, const std::string &k, uint32_t v);
template uint64_t Params::scalarp(const sptr<const Params> &p, const std::string &k, uint64_t v);
template float    Params::scalarp(const sptr<const Params> &p, const std::string &k, float v);
template double   Params::scalarp(const sptr<const Params> &p, const std::string &k, double v);

template v2i  Params::vectorp<int32_t, v2i>(const sptr<const Params> &p, const std::string &k, v2i v);
template v2u  Params::vectorp<uint32_t, v2u>(const sptr<const Params> &p, const std::string &k, v2u v);
template v2f  Params::vectorp<float, v2f>(const sptr<const Params> &p, const std::string &k, v2f v);
template v2d  Params::vectorp<double, v2d>(const sptr<const Params> &p, const std::string &k, v2d v);
template v3f  Params::vectorp<float, v3f>(const sptr<const Params> &p, const std::string &k, v3f v);
template v3d  Params::vectorp<double, v3d>(const sptr<const Params> &p, const std::string &k, v3d v);
template m44f Params::vectorp<float, m44f>(const sptr<const Params> &p, const std::string &k, m44f v);
// clang-format on
