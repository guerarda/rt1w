#ifndef WQUEUE_H
#define WQUEUE_H

#include "types.h"
#include "sptr.hpp"

typedef void (*wqueue_func)(const sptr<Object> &, const sptr<Object> &);

void wqueue_execute(wqueue_func func,
                    const sptr<Object> &obj,
                    const sptr<Object> &arg);
#endif
