#include "rt1w/value.hpp"

#include "rt1w/error.h"

#pragma mark - Scalar Value

template <typename T>
struct scalar_store {
    template <typename U>
    void store(void *p, const T v)
    {
        *(U *)p = (U)v;
    }
    void operator()(vtype_t t, void *p, T v)
    {
        switch (t) {
        case TYPE_INT8: store<int8_t>(p, v); break;
        case TYPE_INT16: store<int16_t>(p, v); break;
        case TYPE_INT32: store<int32_t>(p, v); break;
        case TYPE_INT64: store<int64_t>(p, v); break;
        case TYPE_UINT8: store<uint8_t>(p, v); break;
        case TYPE_UINT16: store<uint16_t>(p, v); break;
        case TYPE_UINT32: store<uint32_t>(p, v); break;
        case TYPE_UINT64: store<uint64_t>(p, v); break;
        case TYPE_FLOAT32: store<float>(p, v); break;
        case TYPE_FLOAT64: store<double>(p, v); break;
        default: memset(p, 0, buffer_type_sizeof(t)); break;
        }
    }
};

template <typename T>
struct _Scalar : Value {
    _Scalar(const void *v) : m_value(*(T *)v) {}

    size_t count() const override { return 1; }
    vtype_t type() const override { return vtype_from_type<T>::value(); }
    void value(vtype_t type, void *v, size_t off, size_t len) const override;

    T m_value;
};

template <typename T>
void _Scalar<T>::value(vtype_t type, void *v, size_t off, size_t len) const
{
    ASSERT(v);
    ASSERT(len > 0);

    if (off == 0) {
        scalar_store<T>()(type, v, m_value);
        off = 1;
    }
    else {
        off = 0;
    }
    if (off < len) {
        size_t sz = buffer_type_sizeof(type);
        memset((uint8_t *)v + off * sz, 0, (len - off) * sz);
    }
}

#pragma mark - Vector Value

template <typename T>
struct vector_store {
    template <typename U>
    void store(void *p, const T *v, size_t n)
    {
        for (size_t i = 0; i < n; i++) {
            ((U *)p)[i] = (U)v[i];
        }
    }
    void operator()(vtype_t t, void *p, const T *v, size_t n)
    {
        switch (t) {
        case TYPE_INT8: store<int8_t>(p, v, n); break;
        case TYPE_INT16: store<int16_t>(p, v, n); break;
        case TYPE_INT32: store<int32_t>(p, v, n); break;
        case TYPE_INT64: store<int64_t>(p, v, n); break;
        case TYPE_UINT8: store<uint8_t>(p, v, n); break;
        case TYPE_UINT16: store<uint16_t>(p, v, n); break;
        case TYPE_UINT32: store<uint32_t>(p, v, n); break;
        case TYPE_UINT64: store<uint64_t>(p, v, n); break;
        case TYPE_FLOAT32: store<float>(p, v, n); break;
        case TYPE_FLOAT64: store<double>(p, v, n); break;
        default: memset(p, 0, buffer_type_sizeof(t)); break;
        }
    }
};

#pragma mark Vector
#define VECTOR_MAX_SIZE 32

template <typename T>
struct _Vector : Value {
    _Vector(const void *v, size_t n) : m_count(n)
    {
        for (size_t i = 0; i < n; i++) {
            m_data[i] = ((T *)v)[i];
        }
    }

    size_t count() const override { return m_count; }
    vtype_t type() const override { return vtype_from_type<T>::value(); }
    void value(vtype_t type, void *v, size_t off, size_t len) const override;

    static const size_t N = VECTOR_MAX_SIZE / sizeof(T);

    T m_data[N];
    size_t m_count;
};

template <typename T>
void _Vector<T>::value(vtype_t type, void *v, size_t off, size_t len) const
{
    ASSERT(v);
    ASSERT(len > 0);

    if (off < N) {
        size_t n = N - off;
        if (n > len) {
            n = len;
        }
        vector_store<T>()(type, v, &m_data[off], n);
        off = n;
    }
    else {
        off = 0;
    }
    if (off < len) {
        size_t sz = buffer_type_sizeof(type);
        memset((uint8_t *)v + off * sz, 0, (len - off) * sz);
    }
}

#pragma mark Big Vector

template <typename T>
struct _BigVector : Value {
    _BigVector(const void *v, size_t n) : m_count(n)
    {
        m_data = std::make_unique<T[]>(n);
        for (size_t i = 0; i < n; i++) {
            m_data[i] = ((T *)v)[i];
        }
    }

    size_t count() const override { return m_count; }
    vtype_t type() const override { return vtype_from_type<T>::value(); }
    void value(vtype_t type, void *v, size_t off, size_t len) const override;

    uptr<T[]> m_data;
    size_t m_count;
};

template <typename T>
void _BigVector<T>::value(vtype_t type, void *v, size_t off, size_t len) const
{
    ASSERT(v);
    ASSERT(len > 0);

    if (off < m_count) {
        size_t n = m_count - off;
        if (n > len) {
            n = len;
        }
        vector_store<T>()(type, v, &m_data[off], n);
        off = n;
    }
    else {
        off = 0;
    }
    if (off < len) {
        size_t sz = buffer_type_sizeof(type);
        memset((uint8_t *)v + off * sz, 0, (len - off) * sz);
    }
}

#pragma mark - Constructors

template <typename T>
sptr<Value> create_value(const void *v, size_t n)
{
    if (n == 1) {
        return std::make_shared<_Scalar<T>>(v);
    }
    if (n * sizeof(T) <= VECTOR_MAX_SIZE) {
        return std::make_shared<_Vector<T>>(v, n);
    }
    return std::make_shared<_BigVector<T>>(v, n);
}

sptr<Value> Value::create(vtype_t t, void *v, size_t n)
{
    ASSERT(v);
    ASSERT(n > 0);

    switch (t) {
    case TYPE_INT8: return create_value<int8_t>(v, n);
    case TYPE_INT16: return create_value<int16_t>(v, n);
    case TYPE_INT32: return create_value<int32_t>(v, n);
    case TYPE_INT64: return create_value<int64_t>(v, n);
    case TYPE_UINT8: return create_value<uint8_t>(v, n);
    case TYPE_UINT16: return create_value<uint16_t>(v, n);
    case TYPE_UINT32: return create_value<uint32_t>(v, n);
    case TYPE_UINT64: return create_value<uint64_t>(v, n);
    case TYPE_FLOAT32: return create_value<float>(v, n);
    case TYPE_FLOAT64: return create_value<double>(v, n);
    default: break;
    }
    return nullptr;
}
