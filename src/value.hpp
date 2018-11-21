#pragma once

#include "geometry.hpp"
#include "sptr.hpp"
#include "types.h"

typedef buffer_type_t vtype_t;

// clang-format off
template <typename T> struct vtype_from_type;
template<> struct vtype_from_type<void>     { static vtype_t value() { return  TYPE_VOID;    } };
template<> struct vtype_from_type<int8_t>   { static vtype_t value() { return  TYPE_INT8;    } };
template<> struct vtype_from_type<int16_t>  { static vtype_t value() { return  TYPE_INT16;   } };
template<> struct vtype_from_type<int32_t>  { static vtype_t value() { return  TYPE_INT32;   } };
template<> struct vtype_from_type<int64_t>  { static vtype_t value() { return  TYPE_INT64;   } };
template<> struct vtype_from_type<uint8_t>  { static vtype_t value() { return  TYPE_UINT8;   } };
template<> struct vtype_from_type<uint16_t> { static vtype_t value() { return  TYPE_UINT16;  } };
template<> struct vtype_from_type<uint32_t> { static vtype_t value() { return  TYPE_UINT32;  } };
template<> struct vtype_from_type<uint64_t> { static vtype_t value() { return  TYPE_UINT64;  } };
template<> struct vtype_from_type<float>    { static vtype_t value() { return  TYPE_FLOAT32; } };
template<> struct vtype_from_type<double>   { static vtype_t value() { return  TYPE_FLOAT64; } };

struct Value : Object {
    static sptr<Value> create(vtype_t t, void *v, size_t n);

    template <typename T>
    static sptr<Value> create(const T *v, size_t n)
    {
        return create(vtype_from_type<T>::value(), (void *)v, n);
    }

    static sptr<Value> i32(int32_t v) { return create<int32_t>(&v, 1); }
    static sptr<Value> i64(int64_t v) { return create<int64_t>(&v, 1); }
    static sptr<Value> u32(uint32_t v) { return create<uint32_t>(&v, 1); }
    static sptr<Value> u64(uint64_t v) { return create<uint64_t>(&v, 1); }
    static sptr<Value> f32(float v) { return create<float>(&v, 1); }
    static sptr<Value> f64(double v) { return create<double>(&v, 1); }

    static sptr<Value> vector2i(v2i v) { return create<int32_t>(&v.x, 2); }
    static sptr<Value> vector2u(v2u v) { return create<uint32_t>(&v.x, 2); }
    static sptr<Value> vector3f(v3f v) { return create<float>(&v.x, 3); }

    static sptr<Value> vector2u(uint32_t x, uint32_t y) { return vector2u({ x, y }); }
    static sptr<Value> vector3f(float x, float y, float z) { return vector3f({ x, y, z }); }

    int32_t i32() const { return scalar<int32_t>(this); }
    int64_t i64() const { return scalar<int64_t>(this); }
    uint32_t u32() const { return scalar<uint32_t>(this); }
    uint64_t u64() const { return scalar<uint64_t>(this); }
    float f32() const { return scalar<float>(this); }
    double f64() const { return scalar<double>(this); }

    v2i vector2i() const { return vector<int32_t, v2i>(this); }
    v2u vector2u() const { return vector<uint32_t, v2u>(this); }
    v3f vector3f() const { return vector<float, v3f>(this); }
    v3d vector3d() const { return vector<double, v3d>(this); }

    virtual vtype_t type() const = 0;
    virtual size_t count() const = 0;
    virtual void value(vtype_t type, void *v, size_t off, size_t len) const = 0;
    // clang-format on

protected:
    template <typename T>
    T scalar(const Value *v) const
    {
        if (v) {
            T rv;
            v->value(vtype_from_type<T>::value(), &rv, 0, 1);
            return rv;
        }
        return T{ 0 };
    }

    template <typename T, typename U>
    U vector(const Value *v) const
    {
        U rv;
        if (v) {
            size_t n = sizeof(U) / sizeof(T);
            v->value(vtype_from_type<T>::value(), &rv, 0, n);
        }
        else {
            memset(&rv, 0, sizeof(rv));
        }
        return rv;
    }
};
