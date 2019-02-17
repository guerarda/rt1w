#pragma once

#include "geometry.hpp"
#include "sptr.hpp"

#include <map>
#include <string>

struct Shape;
struct Texture;
struct Value;

struct Params : Object {
    static sptr<Params> create();

    virtual void insert(const std::string &k, const sptr<Params> &v) = 0;
    virtual void insert(const std::string &k, const std::string &v) = 0;
    virtual void insert(const std::string &k, const sptr<Shape> &v) = 0;
    virtual void insert(const std::string &k, const sptr<Texture> &v) = 0;
    virtual void insert(const std::string &k, const sptr<Value> &v) = 0;

    virtual void merge(const sptr<Params> &p) = 0;
    virtual void merge(const std::map<std::string, sptr<Params>> &p) = 0;
    virtual void merge(const std::map<std::string, std::string> &p) = 0;
    virtual void merge(const std::map<std::string, sptr<Shape>> &p) = 0;
    virtual void merge(const std::map<std::string, sptr<Texture>> &p) = 0;
    virtual void merge(const std::map<std::string, sptr<Value>> &p) = 0;

    virtual sptr<Params> params(const std::string &k) const = 0;
    virtual sptr<Shape> shape(const std::string &k) const = 0;
    virtual std::string string(const std::string &k) const = 0;
    virtual sptr<Texture> texture(const std::string &k) const = 0;
    virtual sptr<Value> value(const std::string &k) const = 0;

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
    static m44f matrix44f(const sptr<const Params> &p, const std::string &k, m44f v) { return vectorp<float, m44f>(p, k, v); }

    static std::string string(const sptr<const Params> &p, const std::string &k, const std::string &v);
    // clang-format on
protected:
    template <typename T>
    static T scalarp(const sptr<const Params> &p, const std::string &k, T v);

    template <typename T, typename U>
    static U vectorp(const sptr<const Params> &p, const std::string &k, U v);
};
