#include "scene.hpp"

#include "camera.hpp"
#include "error.h"
#include "light.hpp"
#include "material.hpp"
#include "params.hpp"
#include "primitive.hpp"
#include "shape.hpp"
#include "texture.hpp"
#include "value.hpp"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/filereadstream.h>

#include <libgen.h>

#include <map>
#include <vector>

#pragma mark Utils

static bool is_absolute_path(const std::string &path)
{
    return !path.empty() && path[0] == '/';
}

static std::string absolute_path(const std::string &path)
{
    char full[PATH_MAX];
    if (realpath(path.c_str(), full)) {
        return std::string(full);
    }
    return path;
}

static std::string resolve_path(const std::string &dir, const std::string &path)
{
    ASSERT(!dir.empty());

    if (path.empty()) {
        return std::string();
    }
    if (is_absolute_path(path)) {
        return path;
    }
    return absolute_path(dir + "/" + path);
}

static sptr<Params> read_params(const rapidjson::Value &v, const std::string &dir)
{
    sptr<Params> p = Params::create();

    for (auto &m : v.GetObject()) {
        std::string name = m.name.GetString();
        if (m.value.IsString()) {
            std::string s = m.value.GetString();
            if (name == "filename") {
                s = resolve_path(dir, s);
            }
            p->insert(name, s);
        }
        else if (m.value.IsArray()) {
            std::vector<double> a;
            for (auto &num : m.value.GetArray()) {
                if (num.IsNumber()) {
                    a.push_back(num.GetDouble());
                }
                else if (num.IsArray()) {
                    for (auto &nnum : num.GetArray()) {
                        ASSERT(nnum.IsNumber());
                        a.push_back(nnum.GetDouble());
                    }
                }
                else {
                    warning("Unsuported array");
                }
            }
            p->insert(name, Value::create(a.data(), a.size()));
        }
        else if (m.value.IsNumber()) {
            sptr<Value> num;
            if (m.value.IsUint()) {
                num = Value::u32(m.value.GetUint());
            }
            else if (m.value.IsInt()) {
                num = Value::i32(m.value.GetInt());
            }
            else if (m.value.IsUint64()) {
                num = Value::u64(m.value.GetUint64());
            }
            else if (m.value.IsInt64()) {
                num = Value::i64(m.value.GetInt64());
            }
            else if (m.value.IsDouble()) {
                num = Value::f64(m.value.GetDouble());
            }
            p->insert(name, num);
        }
        else if (m.value.IsBool()) {
            warning("Bool values are not supported");
        }
        else if (m.value.IsObject()) {
            p->insert(name, read_params(m.value, dir));
        }
        else {
        }
    }
    return p;
}

#pragma mark - Render Description

struct _RenderDesc : RenderDescription {
    _RenderDesc(const std::vector<sptr<Primitive>> &p,
                const std::vector<sptr<Light>> &l,
                const sptr<Camera> &c,
                const sptr<Params> &o) :
        m_primitives(p),
        m_lights(l),
        m_camera(c),
        m_options(o)
    {}

    const std::vector<sptr<Primitive>> &primitives() const override
    {
        return m_primitives;
    }
    const std::vector<sptr<Light>> &lights() const override { return m_lights; }
    sptr<Camera> camera() const override { return m_camera; }
    sptr<const Params> options() const override { return m_options; }

    std::vector<sptr<Primitive>> m_primitives;
    std::vector<sptr<Light>> m_lights;
    sptr<Camera> m_camera;
    sptr<Params> m_options;
};

#pragma mark - Render From JSON

struct _RenderDescFromJSON : RenderDescription {
    _RenderDescFromJSON(const std::string &path) : m_path(path) {}

    const std::vector<sptr<Primitive>> &primitives() const override
    {
        return m_primitives;
    }
    const std::vector<sptr<Light>> &lights() const override { return m_lights; }
    sptr<Camera> camera() const override { return m_camera; }
    sptr<const Params> options() const override { return m_options; }

    int32_t init();

    sptr<Light> read_light(const rapidjson::Value &v) const;
    sptr<Material> read_material(const rapidjson::Value &v) const;
    sptr<Shape> read_shape(const rapidjson::Value &v) const;
    sptr<Texture> read_texture(const rapidjson::Value &v) const;

    void load_camera();
    void load_lights();
    void load_materials();
    void load_options();
    void load_primitives();
    void load_shapes();
    void load_textures();

    std::string m_path;
    std::string m_dir;
    rapidjson::Document m_doc;

    std::vector<sptr<Primitive>> m_primitives;
    std::vector<sptr<Light>> m_lights;
    sptr<Camera> m_camera;
    sptr<Params> m_options;

    std::map<std::string, sptr<Texture>> m_textures;
    std::map<std::string, sptr<Material>> m_materials;
    std::map<std::string, sptr<Shape>> m_shapes;
};

int32_t _RenderDescFromJSON::init()
{
    FILE *fp = fopen(m_path.c_str(), "r");
    char readBuffer[65536];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    rapidjson::ParseResult ok = m_doc.ParseStream<rapidjson::kParseCommentsFlag>(is);
    fclose(fp);

    /* Check that the file was parsed without error */
    if (!ok) {
        error("JSON parse error: %s (%lu)\n", GetParseError_En(ok.Code()), ok.Offset());
        return -1;
    }
    /* Get directory containing the json file that will be used to resolve
     * relative paths */
    char buf[PATH_MAX];
    m_path.copy(buf, m_path.size());

    m_dir = std::string(dirname(buf));

    load_textures();
    load_materials();
    load_shapes();
    load_camera();
    load_options();
    load_primitives();
    load_lights();

    if (!m_primitives.empty() && m_camera) {
        return 0;
    }
    return -1;
}

sptr<Light> _RenderDescFromJSON::read_light(const rapidjson::Value &v) const
{
    sptr<Params> p = read_params(v, m_dir);
    p->merge(m_textures);
    return Light::create(p);
}

sptr<Material> _RenderDescFromJSON::read_material(const rapidjson::Value &v) const
{
    sptr<Params> p = read_params(v, m_dir);
    p->merge(m_textures);
    return Material::create(p);
}

sptr<Shape> _RenderDescFromJSON::read_shape(const rapidjson::Value &v) const
{
    return Shape::create(read_params(v, m_dir));
}

sptr<Texture> _RenderDescFromJSON::read_texture(const rapidjson::Value &v) const
{
    sptr<Params> p = read_params(v, m_dir);
    p->merge(m_textures);
    return Texture::create(p);
}

void _RenderDescFromJSON::load_textures()
{
    auto section = m_doc.FindMember("textures");
    if (section != m_doc.MemberEnd()) {
        for (auto &v : section->value.GetArray()) {
            auto itn = v.FindMember("name");
            if (itn != v.MemberEnd()) {
                std::string k = itn->value.GetString();
                sptr<Texture> tex = read_texture(v);

                if (!k.empty() && tex) {
                    m_textures.insert(std::make_pair(k, tex));
                }
                WARNING_IF(!tex, "Couldn't create texture \"%s\"", k.c_str());
            }
            else {
                warning("Found unamed texture, skipping");
            }
        }
    }
}

void _RenderDescFromJSON::load_materials()
{
    // FIXME: CHECKS
    auto section = m_doc.FindMember("materials");
    if (section != m_doc.MemberEnd()) {
        for (auto &v : section->value.GetArray()) {
            auto itn = v.FindMember("name");
            if (itn != v.MemberEnd()) {
                std::string k = itn->value.GetString();
                sptr<Material> mat = read_material(v);

                WARNING_IF(!mat, "Couldn't create material \"%s\"", k.c_str());
                m_materials.insert(std::make_pair(k, mat));
            }
            else {
                warning("Found unamed material, skipping");
            }
        }
    }
}

void _RenderDescFromJSON::load_shapes()
{
    auto section = m_doc.FindMember("shapes");
    if (section != m_doc.MemberEnd()) {
        for (auto &v : section->value.GetArray()) {
            auto itn = v.FindMember("name");
            if (itn != v.MemberEnd()) {
                std::string k = itn->value.GetString();
                sptr<Shape> shape = read_shape(v);

                WARNING_IF(!shape, "Couldn't create shape \"%s\"", k.c_str());
                m_shapes.insert(std::make_pair(k, shape));
            }
        }
    }
}

void _RenderDescFromJSON::load_lights()
{
    auto section = m_doc.FindMember("lights");
    if (section != m_doc.MemberEnd()) {
        size_t ix = 0;
        for (auto &v : section->value.GetArray()) {
            if (sptr<Light> light = read_light(v)) {
                m_lights.push_back(light);
            }
            else {
                warning("Couldn't create light at index %lu", ix);
            }
        }
    }
}

void _RenderDescFromJSON::load_camera()
{
    auto section = m_doc.FindMember("camera");
    if (section != m_doc.MemberEnd()) {
        m_camera = Camera::create(read_params(section->value, m_dir));
    }
    else {
        error("Missing \"camera\"");
    }
}

void _RenderDescFromJSON::load_options()
{
    auto section = m_doc.FindMember("options");
    if (section != m_doc.MemberEnd()) {
        m_options = read_params(section->value, m_dir);
    }
}

void _RenderDescFromJSON::load_primitives()
{
    auto section = m_doc.FindMember("primitives");
    if (section != m_doc.MemberEnd()) {
        size_t ix = 0;
        for (auto &v : section->value.GetArray()) {
            if (v.IsObject()) {
                auto itf = v.FindMember("file");
                if (itf != v.MemberEnd()) {
                    if (itf->value.IsString()) {
                        std::string file = itf->value.GetString();
                        m_primitives.emplace_back(Primitive::load_obj(file));
                    }
                    warning("Expected filename for primitive at index %lu", ix);
                    continue;
                }

                auto its = v.FindMember("shape");
                auto itm = v.FindMember("material");
                if (its != v.MemberEnd() || itm != v.MemberEnd()) {
                    WARNING_IF(its == v.MemberEnd(),
                               "Primitive at index %lu, no shape found",
                               ix);
                    WARNING_IF(itm == v.MemberEnd(),
                               "Primitive at index %lu, no material found",
                               ix);

                    if (its != v.MemberEnd() && itm != v.MemberEnd()) {
                        sptr<Shape> shape;
                        if (its->value.IsObject()) {
                            shape = read_shape(its->value);
                        }
                        else if (its->value.IsString()) {
                            std::string name = its->value.GetString();
                            shape = m_shapes[name];
                            WARNING_IF(!shape,
                                       "Couldn't find shape named \"%s\"",
                                       name.c_str());
                        }
                        WARNING_IF(!shape, "Primitive at index %lu, invalid shape", ix);

                        sptr<Material> mat;
                        if (itm->value.IsObject()) {
                            mat = read_material(itm->value);
                        }
                        else if (itm->value.IsString()) {
                            std::string name = itm->value.GetString();
                            mat = m_materials[name];
                            WARNING_IF(!mat,
                                       "Couldn't find material named \"%s\"",
                                       name.c_str());
                        }
                        WARNING_IF(!mat, "Primitive at index %lu, invalid material", ix);

                        if (shape && mat) {
                            m_primitives.emplace_back(Primitive::create(shape, mat));
                        }
                        else {
                            warning("Couldnt create Primitive at index %lu", ix);
                        }
                    }
                }
            }
            else {
                warning("Primitive at index %lu must be an object", ix);
            }
            ix++;
        }
    }
    if (m_primitives.empty()) {
        error("Couldn't find a primitive");
    }
}

#pragma mark - Scene

struct _Scene : Scene {
    _Scene(const sptr<Primitive> &w, const std::vector<sptr<Light>> &l) :
        m_world(w),
        m_lights(l)
    {}

    sptr<Primitive> world() const override { return m_world; }
    const std::vector<sptr<Light>> &lights() const override { return m_lights; }

    sptr<Primitive> m_world;
    std::vector<sptr<Light>> m_lights;
};

#pragma mark - Static constructors

sptr<RenderDescription> RenderDescription::create(
    const std::vector<sptr<Primitive>> &primitives,
    const std::vector<sptr<Light>> &lights,
    const sptr<Camera> &camera,
    const sptr<Params> &options)
{
    return std::make_shared<_RenderDesc>(primitives, lights, camera, options);
}

sptr<RenderDescription> RenderDescription::load(const std::string &path)
{
    sptr<_RenderDescFromJSON> render = std::make_shared<_RenderDescFromJSON>(path);

    int32_t err = render->init();
    if (!err) {
        return render;
    }
    error("Couldn't extract a valid render description from %s", path.c_str());
    return nullptr;
}

sptr<Scene> Scene::create(const sptr<Primitive> &world,
                          const std::vector<sptr<Light>> &lights)
{
    return std::make_shared<_Scene>(world, lights);
}
