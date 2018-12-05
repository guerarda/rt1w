#include "image.hpp"

#include "error.h"
#include "imageio.h"
#include "params.hpp"

struct _ImageFile : Image {
    _ImageFile(const std::string &filename);
    ~_ImageFile() override;

    buffer_t buffer() const override { return m_buffer; }
    v2u size() const override { return m_size; }

    std::string m_filename;
    buffer_t m_buffer;
    v2u m_size;
};

_ImageFile::_ImageFile(const std::string &filename)
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

_ImageFile::~_ImageFile()
{
    if (m_buffer.data) {
        free(m_buffer.data);
    }
}

#pragma mark - Static constructor

sptr<Image> Image::create(const std::string &filename)
{
    return std::make_shared<_ImageFile>(filename);
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
