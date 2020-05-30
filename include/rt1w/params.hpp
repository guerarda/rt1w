#pragma once

#include "rt1w/geometry.hpp"
#include "rt1w/sptr.hpp"
#include "rt1w/transform.hpp"

#include <map>
#include <string>
#include <type_traits>

struct Material;
struct Primitive;
struct Shape;
struct Texture;
struct Value;

struct Params : Object {
    static sptr<Params> create();

    /* Returns a param set resulting from merging all the args together */
    template <typename... Args>
    static sptr<Params> create(Args... args)
    {
        auto p = Params::create();
        for (const auto &a : { args... }) {
            p->merge(a);
        }
        return p;
    }

    virtual void insert(const std::string &k, const std::string &v) = 0;
    virtual void insert(const std::string &k, const sptr<Object> &v) = 0;

    virtual void merge(const sptr<Params> &p) = 0;
    virtual void merge(const std::map<std::string, std::string> &p) = 0;
    virtual void merge(const std::map<std::string, sptr<Object>> &p) = 0;

    virtual std::string string(const std::string &k) const = 0;
    virtual sptr<Object> object(const std::string &k) const = 0;

    // clang-format off
    static int32_t i32(const sptr<const Params> &p, const std::string &k, int32_t v) { return scalarp<int32_t>(p, k, v); }
    static int64_t i64(const sptr<const Params> &p, const std::string &k, int64_t v) { return scalarp<int64_t>(p, k, v); }
    static uint32_t u32(const sptr<const Params> &p, const std::string &k, uint32_t v) { return scalarp<uint32_t>(p, k, v); }
    static uint64_t u64(const sptr<const Params> &p, const std::string &k, uint64_t v) { return scalarp<uint64_t>(p, k, v); }
    static float f32(const sptr<const Params> &p, const std::string &k, float v) { return scalarp<float>(p, k, v); }
    static double f64(const sptr<const Params> &p, const std::string &k, double v) { return scalarp<double>(p, k, v); }

    static v2i vector2i(const sptr<const Params> &p, const std::string &k, v2i v) { return vectorp<int32_t, v2i>(p, k, v); }
    static v2u vector2u(const sptr<const Params> &p, const std::string &k, v2u v) { return vectorp<uint32_t, v2u>(p, k, v); }
    static v2f vector2f(const sptr<const Params> &p, const std::string &k, v2f v) { return vectorp<float, v2f>(p, k, v); }
    static v2d vector2d(const sptr<const Params> &p, const std::string &k, v2d v) { return vectorp<double, v2d>(p, k, v); }
    static v3f vector3f(const sptr<const Params> &p, const std::string &k, v3f v) { return vectorp<float, v3f>(p, k, v); }
    static v3d vector3d(const sptr<const Params> &p, const std::string &k, v3d v) { return vectorp<double, v3d>(p, k, v); }
    static m44f matrix44f(const sptr<const Params> &p, const std::string &k, m44f v = m44f_identity()) { return vectorp<float, m44f>(p, k, v); }

    static std::string string(const sptr<const Params> &p, const std::string &k, const std::string &v = {});

    static sptr<Material> material(const sptr<const Params> &p, const std::string &k, const sptr<Material> &v = nullptr) { return object<Material>(p, k, v); }
    static sptr<Primitive> primitive(const sptr<const Params> &p, const std::string &k, const sptr<Primitive> &v = nullptr) { return object<Primitive>(p, k, v); }
    static sptr<Shape> shape(const sptr<const Params> &p, const std::string &k, const sptr<Shape> &v = nullptr) { return object<Shape>(p, k, v); }
    static sptr<Texture> texture(const sptr<const Params> &p, const std::string &k, const sptr<Texture> &v = nullptr) { return object<Texture>(p, k, v); }
    static sptr<Value> value(const sptr<const Params> &p, const std::string &k, const sptr<Value> &v = nullptr) { return object<Value>(p, k ,v); }

    // clang-format on
protected:
    template <typename T>
    static T scalarp(const sptr<const Params> &p, const std::string &k, T v);

    template <typename T, typename U>
    static U vectorp(const sptr<const Params> &p, const std::string &k, U v);

    template <typename T>
    static sptr<T> object(const sptr<const Params> &p,
                          const std::string &k,
                          const sptr<T> &v);
};
