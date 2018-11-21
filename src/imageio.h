#pragma once

#ifdef __cplusplus
#include <cstddef>
#include <cstdint>
#else
#include <stddef.h>
#include <stdint.h>
#endif

struct buffer;

__BEGIN_DECLS

int32_t image_write_png(const char *filename,
                        uint32_t w,
                        uint32_t h,
                        const void *data,
                        size_t bpr);

int32_t image_read_png(const char *filename, struct buffer *buf);
__END_DECLS
