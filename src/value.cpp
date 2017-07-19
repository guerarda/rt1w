#include "value.hpp"
#include <assert.h>

template <typename T> struct vtype_from_type;
template<> struct vtype_from_type<void>        { static vtype_t value() { return  TYPE_VOID;    } };
template<> struct vtype_from_type<int8_t>      { static vtype_t value() { return  TYPE_INT8;    } };
template<> struct vtype_from_type<int16_t>     { static vtype_t value() { return  TYPE_INT16;   } };
template<> struct vtype_from_type<int32_t>     { static vtype_t value() { return  TYPE_INT32;   } };
template<> struct vtype_from_type<int64_t>     { static vtype_t value() { return  TYPE_INT64;   } };
template<> struct vtype_from_type<uint8_t>     { static vtype_t value() { return  TYPE_UINT8;   } };
template<> struct vtype_from_type<uint16_t>    { static vtype_t value() { return  TYPE_UINT16;  } };
template<> struct vtype_from_type<uint32_t>    { static vtype_t value() { return  TYPE_UINT32;  } };
template<> struct vtype_from_type<uint64_t>    { static vtype_t value() { return  TYPE_UINT64;  } };
template<> struct vtype_from_type<float>       { static vtype_t value() { return  TYPE_FLOAT32; } };
template<> struct vtype_from_type<double>      { static vtype_t value() { return  TYPE_FLOAT64; } };

#pragma mark - Scalar Value

template <typename T>
struct scalar_store {
    template <typename U>
    void store(void *p, const T v) { *(U *)p = (U)v; }
    void operator () (vtype_t t, void *p, T v) {
        switch (t) {
        case TYPE_INT8:    store<int8_t>(p, v)  ; break;
        case TYPE_INT16:   store<int16_t>(p, v) ; break;
        case TYPE_INT32:   store<int32_t>(p, v) ; break;
        case TYPE_INT64:   store<int64_t>(p, v) ; break;
        case TYPE_UINT8:   store<uint8_t>(p, v) ; break;
        case TYPE_UINT16:  store<uint16_t>(p, v); break;
        case TYPE_UINT32:  store<uint32_t>(p, v); break;
        case TYPE_UINT64:  store<uint64_t>(p, v); break;
        case TYPE_FLOAT32: store<float>(p, v)   ; break;
        case TYPE_FLOAT64: store<double>(p, v)  ; break;
        default:
            memset(p, 0, buffer_type_sizeof(t));
            break;
        }
    }
};

template <typename T>
struct _Scalar : Value {

    _Scalar(const void *v) : m_value(*(T *)v) { }

    size_t count() const { return 1; }
    vtype_t type() const { return vtype_from_type<T>::value(); }
    void value(vtype_t type, void *v, size_t off, size_t len) const;

    T m_value;
};

template <typename T>
void _Scalar<T>::value(vtype_t type, void *v, size_t off, size_t len) const
{
    assert(v);
    assert(len > 0);
    assert (off < len);

    if (off == 0) {
        scalar_store<T>()(type, v, m_value);
        off = 1;
    } else {
        off = 0;
    }
    if (off < len) {
        size_t sz = buffer_type_sizeof(type);
        memset((uint8_t *)v + off * sz, 0, (len - off) * sz);
    }
}

#pragma mark - Constructors

template <typename T>
sptr<Value> create_value(const void *v, size_t count)
{
    if (count == 1) {
        return std::make_shared<_Scalar<T>>(v);
    }
    return nullptr;
}

sptr<Value> Value::create(vtype_t t, void *v, size_t count)
{
    switch (t) {
    case TYPE_INT8:    return create_value<int8_t>(v, count);
    case TYPE_INT16:   return create_value<int16_t>(v, count);
    case TYPE_INT32:   return create_value<int32_t>(v, count);
    case TYPE_INT64:   return create_value<int64_t>(v, count);
    case TYPE_UINT8:   return create_value<uint8_t>(v, count);
    case TYPE_UINT16:  return create_value<uint16_t>(v, count);
    case TYPE_UINT32:  return create_value<uint32_t>(v, count);
    case TYPE_UINT64:  return create_value<uint64_t>(v, count);
    case TYPE_FLOAT32: return create_value<float>(v, count);
    case TYPE_FLOAT64: return create_value<double>(v, count);
    default:
        break;
    }
    return nullptr;
}
