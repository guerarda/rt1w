#pragma once

#include "geometry.hpp"
#include "sptr.hpp"

#include <map>
#include <string>

struct Texture;
struct Value;

struct Params : Object {
    static sptr<Params> create();

    virtual void insert(const std::string &k, const std::string &v) = 0;
    virtual void insert(const std::string &k, const sptr<Params> &v) = 0;
    virtual void insert(const std::string &k, const sptr<Texture> &v) = 0;
    virtual void insert(const std::string &k, const sptr<Value> &v) = 0;

    virtual void merge(const sptr<Params> &p) = 0;
    virtual void merge(const std::map<const std::string, const std::string> &) = 0;
    virtual void merge(const std::map<const std::string, const sptr<Params>> &) = 0;
    virtual void merge(const std::map<const std::string, const sptr<Texture>> &) = 0;
    virtual void merge(const std::map<const std::string, const sptr<Value>> &) = 0;

    virtual std::string string(const std::string &k) const = 0;
    virtual sptr<Params> params(const std::string &k) const = 0;
    virtual sptr<Value> value(const std::string &k) const = 0;
    virtual sptr<Texture> texture(const std::string &k) const = 0;

    static int32_t i32(const sptr<Params> &p, const std::string &n, int32_t v);
    static int64_t i64(const sptr<Params> &p, const std::string &n, int64_t v);
    static uint32_t u32(const sptr<Params> &p, const std::string &n, uint32_t v);
    static uint64_t u64(const sptr<Params> &p, const std::string &n, uint64_t v);
    static float f32(const sptr<Params> &p, const std::string &n, float v);
    static double f64(const sptr<Params> &p, const std::string &n, double v);

    static v2i vector2i(const sptr<Params> &p, const std::string &n, v2i v);
    static v2u vector2u(const sptr<Params> &p, const std::string &n, v2u v);
    static v3f vector3f(const sptr<Params> &p, const std::string &n, v3f v);
    static v3d vector3d(const sptr<Params> &p, const std::string &n, v3d v);

    static m44f matrix4x4f(const sptr<Params> &p, const std::string &n, m44f v);

    static std::string string(const sptr<Params> &p,
                              const std::string &n,
                              const std::string &v);
};
