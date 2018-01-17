#ifndef SPTR_H
#define SPTR_H

#include <memory>

template <typename T>
using sptr = typename std::shared_ptr<T>;

template <typename T>
using uptr = typename std::unique_ptr<T>;

/* Convenience unique_ptr constructor */
template<typename T, typename ...Args>
std::unique_ptr<T> make_unique(Args &&...args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
/* Empty struct in order to pass sptr<object> around, without
 * having to template everything. Might be a better way to do this...
 */
struct Object {
    virtual ~Object() { }
};

#endif
