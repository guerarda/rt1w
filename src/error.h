#pragma once

#include "types.h"

__BEGIN_DECLS

#ifdef __GNUC__
#define PRINTF_FUNC __attribute__((format (printf, 1, 2)))
#define NORETURN __attribute__((noreturn))
#else
#define PRINTF_FUNC
#define NORETURN
#endif

#define FUNC_IF(FUNC_NAME, EXP, ...) do {       \
        if (bool(EXP)) {                        \
            FUNC_NAME(__VA_ARGS__);             \
        }                                       \
    } while (0);

#define WARNING_IF(EXP, ...) FUNC_IF(warning, EXP, __VA_ARGS__)
#define ERROR_IF(EXP, ...) FUNC_IF(error, EXP, __VA_ARGS__)
#define DIE_IF(EXP, ...) FUNC_IF(die, EXP, __VA_ARGS__)

#ifndef NDEBUG
#define ASSERT(EXP) FUNC_IF(trap, !bool(EXP), "ASSERT FAIL at %s:%d (%s)", __FILE__, __LINE__, #EXP)
#else
#define ASSERT(EXP)
#endif

void warning(const char *fmt, ...) PRINTF_FUNC;
void error(const char *fmt, ...) PRINTF_FUNC;
NORETURN void trap(const char *fmt, ...) PRINTF_FUNC;
NORETURN void die(const char *fmt, ...) PRINTF_FUNC;

__END_DECLS
