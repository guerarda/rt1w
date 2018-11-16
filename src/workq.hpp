#pragma once

#include "sptr.hpp"
#include "types.h"

struct workq;
struct Event;

typedef void (*workq_func)(const sptr<Object> &, const sptr<Object> &);

/*!
 * @brief Returns an unspecified work queue.
 */
struct workq *workq_get_queue();

/*!
 * @brief Request the function func to be called on the specified
 * work queue. If workq is NULL then the function will be called
 * immediately on the current thread. Both obj & arg are retained
 * until the command is executed.
 * @param wqeue The work queue on which the function will be executed.
 * @param func The function to execute: func(obj, arg).
 * @param obj The first argument to func.
 * @param arg The second argument to func.
 * @returns An event that signals when the func has been executed.
 */
sptr<Event> workq_execute(struct workq *workq,
                          workq_func func,
                          const sptr<Object> &obj,
                          const sptr<Object> &arg);
