#include "imageio.h"
#include "types.h"
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

int32_t image_read_png(const char *filename, struct buffer *buf)
{
    assert(filename);
    assert(buf);

    int32_t err = 0;
    FILE *fp = NULL;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;

    fp = fopen(filename, "rb");
    if (!fp) {
        err = 1;
        goto clean;
    }

    /* Check signature */
    uint8_t sig[8];
    fread(sig, 1, 8, fp);
    if (!png_check_sig(sig, 8)) {
        err = 1;
        goto clean;
    }

    /* Init read struct */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
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

    /* Error handling */
    if (setjmp(png_jmpbuf(png_ptr))) {
        err = 1;
        goto clean;
    }

    /* Read info */
    uint8_t bit_depth  = 0;
    uint8_t color_type = 0;
    uint8_t channels   = 0;
    uint32_t width     = 0;
    uint32_t height    = 0;

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);

    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);
    bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    color_type = png_get_color_type(png_ptr, info_ptr);
    channels = png_get_channels(png_ptr, info_ptr);

    /* Alloc buffers */
    size_t bpr = png_get_rowbytes(png_ptr, info_ptr);
    uint8_t *ptr = (uint8_t *)malloc(height * bpr);
    png_bytep *row_ptr = (png_bytep *)malloc(height * sizeof(png_bytep));

    if (!ptr) {
        err = 1;
        goto clean;
    }
    if (!row_ptr) {
        err = 1;
        free(ptr);
        goto clean;
    }

    for (uint32_t i = 0; i < height; i++) {
        row_ptr[i] = ptr + i * bpr;
    }
    /* End read */
    png_read_image(png_ptr, row_ptr);
    png_read_end(png_ptr, NULL);
    free(row_ptr);

    /* Write info & data to struct buffer */
    buf->data = ptr;
    buf->rect.org.x = 0;
    buf->rect.org.y = 0;
    buf->rect.size.x = width;
    buf->rect.size.y = height;
    buf->bpr = bpr;

    switch(color_type) {
    case PNG_COLOR_TYPE_RGB:
        buf->format.order = ORDER_RGB;
        break;
    case PNG_COLOR_TYPE_RGBA:
        buf->format.order = ORDER_RGBA;
        break;
    default:
        err = 1;
        goto clean;
    }

    switch (bit_depth) {
    case 8:
        buf->format.type = TYPE_UINT8;
        break;
    case 16:
        buf->format.type = TYPE_UINT16;
        break;
    default:
        err = 1;
        goto clean;
    }
    buf->format.size = channels * bit_depth / 8;

    /* Cleanup */
clean:
    if (fp) {
        fclose(fp);
    }
    if (png_ptr) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
    }
    if (info_ptr) {
        png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    }

    return err;
}
