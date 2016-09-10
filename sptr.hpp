#ifndef SPTR_H
#define SPTR_H

#include <memory>

template<typename T>
using sptr = typename std::shared_ptr<T>;

#endif
