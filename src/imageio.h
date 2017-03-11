#ifndef IMAGEIO_H
#define IMAGEIO_H

#include <stdint.h>
#include <stddef.h>

__BEGIN_DECLS

int32_t image_write_png(const char *filename,
                        uint32_t w,
                        uint32_t h,
                        const void *data,
                        size_t bpr);
__END_DECLS

#endif
