#include "error.h"
#include <string>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"

static void process_error(const char *fmt, va_list args, const char *type)
{
    va_list argscpy;
    va_copy(argscpy, args);

    size_t size = (size_t)vsnprintf(nullptr, 0, fmt, args) + 1;

    std::string str;
    str.resize(size);
    vsnprintf(&str[0], size, fmt, argscpy);
    str.pop_back();
    va_end(argscpy);

    std::string error_str(type);
    if (!error_str.empty()) {
        error_str.append(": ");
    }
    error_str.append(str);
    error_str.append("\n");

    fprintf(stderr, "%s", error_str.c_str());
}

#pragma clang diagnostic pop

void warning(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    process_error(fmt, args, "Warning");
    va_end(args);
}

void error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    process_error(fmt, args, "Error");
    va_end(args);
}

void die(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    process_error(fmt, args, "Fatal");
    va_end(args);
    exit(EXIT_FAILURE);
}

void trap(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    process_error(fmt, args, "");
    va_end(args);

#ifdef _WIN32
    __debugbreak();
#else
    __asm__ __volatile__ ("int3");
#endif
    exit(EXIT_FAILURE);
}
