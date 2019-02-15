#include "image.hpp"

#include "error.h"
#include "imageio.h"
#include "params.hpp"

#pragma mark - Image File

struct ImageFile : Image {
    ImageFile(const std::string &filename);
    ~ImageFile() override
    {
        if (m_buffer.data) {
            free(m_buffer.data);
        }
    }

    buffer_t buffer() override { return m_buffer; }
    v2u size() const override { return m_size; }

    std::string m_filename;
    buffer_t m_buffer;
    v2u m_size;
};

ImageFile::ImageFile(const std::string &filename)
{
    m_filename = filename;
    int32_t err = image_read_png(filename.c_str(), &m_buffer);

    if (!err) {
        m_size = { m_buffer.rect.size.x, m_buffer.rect.size.y };
    }
    else {
        warning("Couldn't load image at \"%s\"", filename.c_str());
    }
}

#pragma mark - Image Convert

template <typename T, typename U>
float Scale();

template <typename T, typename U>
void ConvertPixel(void *dst, const void *src)
{
    for (size_t i = 0; i < 3; ++i) {
        ((T *)dst)[i] = (T)(Scale<T, U>() * ((U *)src)[i]);
    }
}

// clang-format off
template <> float Scale<uint8_t, uint16_t>() { return 1.f / 255.f; }
template <> float Scale<uint8_t, float>()    { return 255.f; }
template <> float Scale<uint16_t, uint8_t>() { return 255.f; }
template <> float Scale<uint16_t, float>()   { return 65535.f; }
template <> float Scale<float, uint8_t>()    { return 1.f / 255.f; }
template <> float Scale<float, uint16_t>()   { return 1.f / 65535.f; }
// clang-format on

struct ImageConvert : Image {
    ImageConvert(const sptr<Image> &image, buffer_format_t fmt) :
        m_img(image),
        m_format(fmt)
    {
        buffer_t b = m_img->buffer();

        ASSERT(b.format.order == fmt.order);

        /* Alloc Buffer */
        m_buffer.rect = b.rect;
        m_buffer.format = m_format;
        m_buffer.bpr = m_buffer.rect.size.x * m_buffer.format.size;
        m_buffer.data = malloc(m_buffer.rect.size.y * m_buffer.bpr);

        /* Check if a conversion is needed */
        if (m_format.type == b.format.type) {
            memcpy(m_buffer.data, b.data, m_buffer.rect.size.y * m_buffer.bpr);
            return;
        }

        /* Convert */
        buffer_type_t srcType = b.format.type;
        buffer_type_t dstType = m_buffer.format.type;
        size_t nx = m_img->size().x;
        size_t ny = m_img->size().y;

        for (size_t y = 0; y < ny; ++y) {
            const uint8_t *sp = (const uint8_t *)b.data + y * b.bpr;
            uint8_t *dp = (uint8_t *)m_buffer.data + y * m_buffer.bpr;
            for (size_t x = 0; x < nx; ++x) {
                if (srcType == TYPE_UINT8) {
                    if (dstType == TYPE_UINT16) {
                        ConvertPixel<uint16_t, uint8_t>((void *)dp, (const void *)sp);
                    }
                    else if (dstType == TYPE_FLOAT32) {
                        ConvertPixel<float, uint8_t>((void *)dp, (const void *)sp);
                    }
                }
                else if (srcType == TYPE_UINT16) {
                    if (dstType == TYPE_UINT8) {
                        ConvertPixel<uint8_t, uint16_t>((void *)dp, (const void *)sp);
                    }
                    else if (dstType == TYPE_FLOAT32) {
                        ConvertPixel<float, uint16_t>((void *)dp, (const void *)sp);
                    }
                }
                else if (srcType == TYPE_FLOAT32) {
                    if (dstType == TYPE_UINT8) {
                        ConvertPixel<uint8_t, float>((void *)dp, (const void *)sp);
                    }
                    else if (dstType == TYPE_UINT16) {
                        ConvertPixel<uint16_t, float>((void *)dp, (const void *)sp);
                    }
                }
                dp += m_buffer.format.size;
                sp += b.format.size;
            }
        }
    }

    ~ImageConvert() override
    {
        if (m_buffer.data) {
            free(m_buffer.data);
        }
    }

    buffer_t buffer() override { return m_buffer; }
    v2u size() const override { return m_img->size(); }

    sptr<Image> m_img;
    buffer_format_t m_format;
    buffer_t m_buffer;
};

#pragma mark - Static constructor

sptr<Image> Image::create(const std::string &filename)
{
    return std::make_shared<ImageFile>(filename);
}

sptr<Image> Image::create(const sptr<Image> &img, buffer_format_t fmt)
{
    return std::make_shared<ImageConvert>(img, fmt);
}

sptr<Image> Image::create(const sptr<Params> &p)
{
    std::string filename = p->string("filename");
    if (!filename.empty()) {
        return Image::create(filename);
    }
    warning("Image parameter \"filename\" is not specified");

    return nullptr;
}
