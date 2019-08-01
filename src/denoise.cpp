#include "denoise.hpp"

#include "error.h"

#ifdef RT1W_WITH_OIDN

#include "event.hpp"
#include "image.hpp"
#include "types.h"
#include "workq.hpp"

#include <OpenImageDenoise/oidn.hpp>

#include <atomic>
#include <memory>

static void denoise(const sptr<Object> &obj, const sptr<Object> &);

struct ImageDenoise : Image, std::enable_shared_from_this<ImageDenoise> {
    ImageDenoise(const sptr<Image> &color,
                 const sptr<Image> &normals,
                 const sptr<Image> &albedo) :
        m_color(color),
        m_normals(normals),
        m_albedo(albedo),
        m_scheduled(0)
    {}

    sptr<Event> schedule() override;
    buffer_t buffer() override
    {
        schedule()->wait();
        return m_buffer;
    }
    v2u size() const override { return m_color->size(); }

    sptr<Image> m_color;
    sptr<Image> m_normals;
    sptr<Image> m_albedo;
    sptr<Event> m_event;
    buffer_t m_buffer;
    std::atomic<int32_t> m_scheduled;
};

sptr<Event> ImageDenoise::schedule()
{
    while (m_scheduled.load() != 1) {
        int32_t expected = 0;
        if (m_scheduled.compare_exchange_strong(expected, -1)) {
            std::vector<sptr<Event>> v;
            v.push_back(m_color->schedule());
            if (m_normals) {
                v.push_back(m_normals->schedule());
            }
            if (m_albedo) {
                v.push_back(m_albedo->schedule());
            }
            auto e = Event::create(v);
            m_event = e->notify(workq_get_queue(), denoise, shared_from_this(), {});
            m_scheduled.store(1);
        }
    }
    return m_event;
}

static void denoise(const sptr<Object> &obj, const sptr<Object> &)
{
    auto img = std::static_pointer_cast<ImageDenoise>(obj);
    ASSERT(obj);

    buffer_t b = img->m_color->buffer();
    buffer_t *outb = &img->m_buffer;

    /* Alloc output */
    outb->format = b.format;
    outb->rect = b.rect;
    outb->bpr = b.bpr;
    outb->data = malloc(outb->bpr * outb->rect.size.y);

    v2u size = img->m_color->size();

    oidn::DeviceRef device = oidn::newDevice();
    device.commit();

    oidn::FilterRef filter = device.newFilter("RT");
    filter.setImage("color",
                    img->m_color->buffer().data,
                    oidn::Format::Float3,
                    size.x,
                    size.y);
    if (img->m_normals && img->m_albedo) {
        filter.setImage("normals",
                        img->m_normals->buffer().data,
                        oidn::Format::Float3,
                        size.x,
                        size.y);
        filter.setImage("albedo",
                        img->m_albedo->buffer().data,
                        oidn::Format::Float3,
                        size.x,
                        size.y);
    }
    filter.setImage("output", outb->data, oidn::Format::Float3, size.x, size.y);
    filter.set("srgb", true);
    filter.commit();
    filter.execute();

    const char *errorMessage;
    WARNING_IF(device.getError(errorMessage) != oidn::Error::None, "%s", errorMessage);
}

#pragma mark - Static Constructors

sptr<Image> Denoise(const sptr<Image> &color)
{
    return Denoise(color, nullptr, nullptr);
}

sptr<Image> Denoise(const sptr<Image> &color,
                    const sptr<Image> &normals,
                    const sptr<Image> &albedo)
{
    return color ? std::make_shared<ImageDenoise>(color, normals, albedo) : nullptr;
}

#else

sptr<Image> Denoise(const sptr<Image> &color)
{
    warning("RT1W was built without OpenImageDenoise.");
    return color;
}

sptr<Image> Denoise(const sptr<Image> &color, const sptr<Image> &, const sptr<Image> &)
{
    warning("RT1W was built without OpenImageDenoise support");
    return color;
}
#endif
