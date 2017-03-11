#include "imageio.h"
#include <assert.h>
#include <png.h>

int32_t image_write_png(const char *filename,
                        uint32_t w,
                        uint32_t h,
                        const void *data,
                        size_t bpr)
{
    assert(filename);
    assert(w > 0);
    assert(h > 0);
    assert(data);
    assert(bpr > 0);

    int32_t err = 0;
    FILE *fp = NULL;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;

    fp = fopen(filename, "wb");
    if (!fp) {
        err = 1;
        goto clean;
    }

    /* Init write struct */
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                      NULL,
                                      NULL,
                                      NULL);
    if (!png_ptr) {
        err = 1;
        goto clean;
    }

    /* Init info struct */
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        err = 1;
        goto clean;
    }

    /* Exception handling */
    if (setjmp(png_jmpbuf(png_ptr))) {
        err = 1;
        goto clean;
    }

    /* Set info */
    png_init_io(png_ptr, fp);
    png_set_IHDR(png_ptr,
                 info_ptr,
                 w,
                 h,
                 8,
                 PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE,
                 PNG_FILTER_TYPE_BASE);
    png_write_info(png_ptr, info_ptr);

    /* Write */
    const uint8_t *sp = (const uint8_t *)data;
    for (uint32_t i = 0; i < h; i++) {
        png_write_row(png_ptr, sp);
        sp += bpr;
    }

    /* End */
    png_write_end(png_ptr, NULL);

clean:
    if (fp) {
        fclose(fp);
    }
    if (info_ptr) {
        png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    }
    if (png_ptr) {
        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    }
    return err;
}
