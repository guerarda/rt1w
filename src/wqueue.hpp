#ifndef WQUEUE_H
#define WQUEUE_H

#include "types.h"
#include "sptr.hpp"

struct wqueue;
struct event;

typedef void (*wqueue_func)(const sptr<Object> &, const sptr<Object> &);

/*!
 * @brief Returns an unspecified work queue.
 */
struct wqueue *wqueue_get_queue();

/*!
 * @brief Request the function func to be called on the specified
 * work queue. If wqueue is NULL then the function will be called
 * immediately on the current thread. Both obj & arg are retained
 * until the command is executed.
 * @param wqeue The work queue on which the function will be executed.
 * @param func The function to execute: func(obj, arg).
 * @param obj The first argument to func.
 * @param arg The second argument to func.
 * @returns An event that signals when the func has been executed.
 */
sptr<event> wqueue_execute(struct wqueue *wqueue,
                           wqueue_func func,
                           const sptr<Object> &obj,
                           const sptr<Object> &arg);
#endif
