#include "rt1w/params.hpp"
#include <memory>
#include <type_traits>

#include "rt1w/material.hpp"
#include "rt1w/primitive.hpp"
#include "rt1w/shape.hpp"
#include "rt1w/texture.hpp"
#include "rt1w/value.hpp"

// clang-format off
template <typename T> struct ptype { typedef T type; };
template <> struct ptype<std::string> { typedef std::string type; };
template <> struct ptype<Params> { typedef sptr<Params> type; };
template <> struct ptype<Shape> { typedef sptr<Shape> type; };
template <> struct ptype<Texture> { typedef sptr<Texture> type; };
template <> struct ptype<Value> { typedef sptr<Value> type; };
template <> struct ptype<Object> { typedef sptr<Object> type; };

struct _Params : Params {
    void insert(const std::string &k, const std::string &v) override { insert<std::string>(k, v); }
    void insert(const std::string &k, const sptr<Object> &v) override { insert<Object>(k,v); }

    void merge(const sptr<Params> &p) override;
    void merge(const std::map<std::string, std::string> &m) override  { merge<std::string>(m); }
    void merge(const std::map<std::string, sptr<Object>> &m) override { merge<Object>(m); }

    std::string string(const std::string &k) const override  { return find<std::string>(k); }
    sptr<Object> object(const std::string &k) const override { return find_recursive<Object>(k); }

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

    sptr<Value> value(const std::string &k)
    {
        if (auto v = std::dynamic_pointer_cast<Value>(object(k))) {
            return v;
        }
        return nullptr;
    }
    std::map<std::string, std::string> m_strings;
    std::map<std::string, sptr<Object>> m_objects;
};

void _Params::merge(const sptr<Params> &p)
{
    if (p) {
        if (sptr<_Params> other = std::static_pointer_cast<_Params>(p)) {
            merge(other->m_strings);
            merge(other->m_objects);
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
    if (auto value = std::dynamic_pointer_cast<Value>(p->object(k))) {
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
    if (auto value = std::dynamic_pointer_cast<Value>(p->object(k))) {
        U rv;
        value->value((T *)&rv, 0, sizeof(U) / sizeof(T));
        return rv;
    }
    return v;
}

/* Enable a different template function for the type Value.
 * Value doesn't have a constructor from a Params object, so the first version won't
 * instantiate correctly */

template <typename T, std::enable_if_t<!std::is_base_of<Value, T>::value> * = nullptr>
sptr<T> fetch_object(const sptr<const Params> &p, const std::string &k)
{
    if (auto obj = p->object(k)) {
        if (auto t = std::dynamic_pointer_cast<T>(obj)) {
            return t;
        }
        if (auto tp = std::dynamic_pointer_cast<Params>(obj)) {
            return T::create(tp);
        }
        ERROR("Params: Unexpected type when retrieving object named \"%s\"", k.c_str());
    }
    return nullptr;
}

template <typename T, std::enable_if_t<std::is_base_of<Value, T>::value> * = nullptr>
sptr<T> fetch_object(const sptr<const Params> &p, const std::string &k)
{
    if (auto obj = p->object(k)) {
        if (auto t = std::dynamic_pointer_cast<T>(obj)) {
            return t;
        }
        ERROR("Params: Unexpected type when retrieving value named \"%s\"", k.c_str());
    }
    return nullptr;
}

template <typename T>
sptr<T> Params::object(const sptr<const Params> &p,
                       const std::string &k,
                       const sptr<T> &v)
{
    ASSERT(p);
    if (auto obj = fetch_object<T>(p, k)) {
        return obj;
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
template <> std::map<std::string, ptype<Object>::type>      &_Params::pmap<Object>()      { return m_objects; }

template <> const std::map<std::string, ptype<std::string>::type> &_Params::const_pmap<std::string>() const { return m_strings; }
template <> const std::map<std::string, ptype<Object>::type>      &_Params::const_pmap<Object>() const      { return m_objects; }

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

template sptr<Material> Params::object(const sptr<const Params> &p, const std::string &k, const sptr<Material> &v);
template sptr<Primitive> Params::object(const sptr<const Params> &p, const std::string &k, const sptr<Primitive> &v);
template sptr<Shape> Params::object(const sptr<const Params> &p, const std::string &k, const sptr<Shape> &v);
template sptr<Texture> Params::object(const sptr<const Params> &p, const std::string &k, const sptr<Texture> &v);
template sptr<Value> Params::object(const sptr<const Params> &p, const std::string &k, const sptr<Value> &v);
// clang-format on
