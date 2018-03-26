#include "scene.hpp"

#include <fstream>
#include <iostream>
#include <assert.h>
#include <map>
#include <vector>

#include "error.h"
#include "primitive.hpp"
#include "camera.hpp"
#include "params.hpp"
#include "texture.hpp"
#include "material.hpp"
#include "value.hpp"

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/error/en.h"


struct _Scene : Scene {

    std::vector<sptr<Primitive>> primitives() const { return m_primitives; }
    sptr<Camera>                 camera() const     { return m_camera; }
    sptr<const Params>           options() const    { return m_options; }

    int32_t init_with_json(const std::string &path);

    std::vector<sptr<Primitive>> m_primitives;
    sptr<Camera>                 m_camera;
    sptr<Params>                 m_options;

    std::map<const std::string, const sptr<Texture>>  m_textures;
    std::map<const std::string, const sptr<Material>> m_materials;
    std::map<const std::string, const sptr<Shape>>    m_shapes;
};

static sptr<Params> read_params(const rapidjson::Value &v)
{
    sptr<Params> p = Params::create();

    for (auto &m : v.GetObject()) {
        std::string name = m.name.GetString();
        if (m.value.IsString()) {
            p->insert(name, m.value.GetString());
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
            p->insert(name, read_params(m.value));
        }
        else {

        }
    }
    return p;
}

static sptr<Texture> read_texture(const rapidjson::Value &v,
                                  std::map<const std::string,
                                  const sptr<Texture>> &textures)
{
    sptr<Params> p = read_params(v);
    p->merge(textures);
    return Texture::create(p);
}

static sptr<Material> read_material(const rapidjson::Value &v,
                                    std::map<const std::string,
                                    const sptr<Texture>> &textures)
{
    sptr<Params> p = read_params(v);
    p->merge(textures);
    return Material::create(p);
}

static sptr<Shape> read_shape(const rapidjson::Value &v)
{
    return Shape::create(read_params(v));
}

static sptr<Camera> read_camera(const rapidjson::Value &v)
{
    return Camera::create(read_params(v));
}

int32_t _Scene::init_with_json(const std::string &path)
{
    FILE *fp = fopen(path.c_str(), "r");
    char readBuffer[65536];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    rapidjson::Document d;
    rapidjson::ParseResult ok = d.ParseStream(is);
    fclose(fp);

    /* Check that the file was parsed without error */
    if (!ok) {
        error("JSON parse error: %s (%lu)\n",
              GetParseError_En(ok.Code()), ok.Offset());
        return -1;
    }

    int32_t err = 0;
    rapidjson::Value::ConstMemberIterator it;

    /* Scene */
    it = d.FindMember("textures");
    if (it != d.MemberEnd()) {
        for (auto &v : it->value.GetArray()) {
            auto itn = v.FindMember("name");
            if (itn != v.MemberEnd()) {
                std::string k = itn->value.GetString();
                sptr<Texture> tex = read_texture(v, m_textures);

                WARNING_IF(!tex, "Couldn't create texture \"%s\"", k.c_str());
                m_textures.insert(std::make_pair(k, tex));
            } else {
                warning("Found unamed texture, skipping");
            }
        }
    }

    it = d.FindMember("materials");
    if (it != d.MemberEnd()) {
        for (auto &v : it->value.GetArray()) {
            auto itn = v.FindMember("name");
            if (itn != v.MemberEnd()) {
                std::string k = itn->value.GetString();
                sptr<Material> mat = read_material(v, m_textures);

                WARNING_IF(!mat, "Couldn't create material \"%s\"", k.c_str());
                m_materials.insert(std::make_pair(k, mat));
            } else {
                warning("Found unamed material, skipping");
            }
        }
    }

    it = d.FindMember("shapes");
    if (it != d.MemberEnd()) {
        for (auto &v : it->value.GetArray()) {
            auto itn = v.FindMember("name");
            if (itn != v.MemberEnd()) {
                std::string k = itn->value.GetString();
                sptr<Shape> shape = read_shape(v);

                WARNING_IF(!shape, "Couldn't create shape \"%s\"", k.c_str());
                m_shapes.insert(std::make_pair(k, shape));
            }
        }
    }

    it = d.FindMember("primitives");
    if (it != d.MemberEnd()) {
        size_t ix = 0;
        for (auto &v : it->value.GetArray()) {
            auto its = v.FindMember("shape");
            auto itm = v.FindMember("material");

            WARNING_IF(its == v.MemberEnd(),
                       "Primitive at index %lu, no shape found", ix);
            WARNING_IF(itm == v.MemberEnd(),
                       "Primitive at index %lu, no material found", ix);

            if (its != v.MemberEnd() && itm != v.MemberEnd()) {
                sptr<Shape> shape;
                if (its->value.IsObject()) {
                    shape = read_shape(its->value);
                }
                else if (its->value.IsString()) {
                    std::string name = its->value.GetString();
                    shape = m_shapes[name];
                    WARNING_IF(!shape, "Couldn't find shape named \"%s\"", name.c_str())
                }
                WARNING_IF(!shape, "Primitive at index %lu, invalid shape", ix);

                sptr<Material> mat;
                if (itm->value.IsObject()) {
                    mat = read_material(itm->value, m_textures);
                }
                else if (itm->value.IsString()) {
                    std::string name = itm->value.GetString();
                    mat = m_materials[name];
                    WARNING_IF(!mat, "Couldn't find material named \"%s\"", name.c_str())
                }
                WARNING_IF(!mat, "Primitive at index %lu, invalid material", ix);

                if (shape && mat) {
                    m_primitives.emplace_back(Primitive::create(shape, mat));
                } else {
                    warning("Couldnt create Primitive at index %lu", ix);
                }
            }
            ix++;
        }
    }
    if (m_primitives.empty()) {
        error("Couldn't find a primitive in %s", path.c_str());
        err = -1;
    }

    /* Camera */
    it = d.FindMember("camera");
    if (it != d.MemberEnd()) {
        m_camera = read_camera(it->value);
    } else {
        error("Missing \"camera\" in %s", path.c_str());
        err = -1;
    }

    /* Rendering options */
    it = d.FindMember("options");
    if (it != d.MemberEnd()) {
        m_options = read_params(it->value);
    }
    return err;
}

sptr<Scene> Scene::create_json(const std::string &path)
{
    sptr<_Scene> scene = std::make_shared<_Scene>();
    int32_t err = scene->init_with_json(path);

    if (!err) {
        return scene;
    }

    error("Couldn't extract a valid scene from %s", path.c_str());

    return nullptr;
}
