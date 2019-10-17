#pragma once

#ifdef __cplusplus
#include <cstdarg>
#include <cstdlib>

#else
#include <stdarg.h>
#include <stdlib.h>
#endif
__BEGIN_DECLS

#ifdef __GNUC__
#define LOG_FUNC __attribute__((format(printf, 2, 3)))
#define PRINTF_FUNC __attribute__((format(printf, 1, 2)))
#define NORETURN __attribute__((noreturn))
#else
#define PRINTF_FUNC
#define NORETURN
#endif

enum LogLevel {
    LOG_LEVEL_INFO = 0,
    LOG_LEVEL_WARNING = 1,
    LOG_LEVEL_ERROR = 2,
    LOG_LEVEL_NUM
};

#define FUNC_IF(FUNC_NAME, EXP, ...) \
    do {                             \
        if (bool(EXP)) {             \
            FUNC_NAME(__VA_ARGS__);  \
        }                            \
    } while (0);

#define LOG_LEVEL_IF(LVL, EXP, ...)    \
    do {                               \
        if (bool(EXP)) {               \
            log_msg(LVL, __VA_ARGS__); \
        }                              \
    } while (0);

#define LOG(...) log_msg(LOG_LEVEL_INFO, __VA_ARGS__);
#define WARNING(...) log_msg(LOG_LEVEL_WARNING, __VA_ARGS__);
#define ERROR(...) log_msg(LOG_LEVEL_ERROR, __VA_ARGS__);

#define LOG_IF(EXP, ...) LOG_LEVEL_IF(LOG_LEVEL_INFO, EXP, __VA_ARGS__)
#define WARNING_IF(EXP, ...) LOG_LEVEL_IF(LOG_LEVEL_WARNING, EXP, __VA_ARGS__)
#define ERROR_IF(EXP, ...) LOG_LEVEL_IF(LOG_LEVEL_ERROR, EXP, __VA_ARGS__)
#define DIE_IF(EXP, ...) FUNC_IF(die, EXP, __VA_ARGS__)

#ifndef NDEBUG
#define ASSERT(EXP) \
    FUNC_IF(trap, !bool(EXP), "ASSERT FAIL at %s:%d (%s)", __FILE__, __LINE__, #EXP)
#else
#define ASSERT(EXP)
#endif

void log_msg(int severity, const char *fmt, ...) LOG_FUNC;
NORETURN void trap(const char *fmt, ...) PRINTF_FUNC;
NORETURN void die(const char *fmt, ...) PRINTF_FUNC;

__END_DECLS
