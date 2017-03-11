#ifndef SPTR_H
#define SPTR_H

#include <memory>

template<typename T>
using sptr = typename std::shared_ptr<T>;

/* Empty struct in order to pass sptr<object> around, without
 *having to template everything. Might be a better way to do this...
 */
struct Object {
    virtual ~Object() { }
};

#endif
