#include "scene.hpp"

#include <fstream>
#include <iostream>
#include <assert.h>
#include <map>
#include <vector>

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

    sptr<Primitive>    primitive() const { return m_primitive; }
    sptr<Camera>       camera() const    { return m_camera; }
    sptr<const Params> options() const   { return m_options; }

    void init_with_json(const std::string &path);

    sptr<Primitive> m_primitive;
    sptr<Camera>    m_camera;
    sptr<Params>    m_options;

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
                assert(num.IsNumber() && "Number expected\n");
                a.push_back(num.GetDouble());
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

        }
        else if (m.value.IsObject()) {

        }
        else {

        }
    }
    return p;
}

static sptr<Texture> read_texture(const rapidjson::Value &v,
                                  std::map<const std::string, const sptr<Texture>> &textures)
{
    sptr<Params> p = read_params(v);
    p->merge(textures);
    return Texture::create(p);
}

static sptr<Material> read_material(const rapidjson::Value &v,
                                    std::map<const std::string, const sptr<Texture>> &textures)
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

void _Scene::init_with_json(const std::string &path)
{
    FILE *fp = fopen(path.c_str(), "r");
    char readBuffer[65536];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    rapidjson::Document d;
    rapidjson::ParseResult ok = d.ParseStream(is);
    fclose(fp);

    /* Check that the file was parsed without error */
    if (!ok) {
        // LOG
        fprintf(stderr,
                "JSON parse error: %s (%lu)\n",
                GetParseError_En(ok.Code()), ok.Offset());
        return;
    }

    rapidjson::Value::ConstMemberIterator it;

    /* Scene */
    it = d.FindMember("textures");
    if (it != d.MemberEnd()) {
        for (auto &v : it->value.GetArray()) {
            // Check object
            std::string k = v.GetObject()["name"].GetString();
            sptr<Texture> tex = read_texture(v, m_textures);
            m_textures.insert(std::make_pair(k, tex));
        }
    } else {
        // LOG
    }

    it = d.FindMember("materials");
    if (it != d.MemberEnd()) {
        for (auto &v : it->value.GetArray()) {
            // Check object
            std::string k = v.GetObject()["name"].GetString();

            sptr<Material> mat = read_material(v, m_textures);
            m_materials.insert(std::make_pair(k, mat));
        }
    } else {
        // LOG
    }

    it = d.FindMember("shapes");
    if (it != d.MemberEnd()) {
        for (auto &v : it->value.GetArray()) {
            // Check object
            std::string k = v.GetObject()["name"].GetString();
            sptr<Shape> shape = read_shape(v);
            m_shapes.insert(std::make_pair(k, shape));
        }
    } else {
        // LOG
    }

    it = d.FindMember("primitives");
    if (it != d.MemberEnd()) {
        std::vector<sptr<Primitive>> cmds;
        for (auto &v : it->value.GetArray()) {
            // Check object
            std::string shape = v.GetObject()["shape"].GetString();
            std::string mat = v.GetObject()["material"].GetString();

            cmds.emplace_back(Primitive::create(m_shapes[shape],
                                                m_materials[mat]));
        }
        m_primitive = Primitive::create(cmds);
    } else {
        // LOG ERROR
        // FAIL
    }

    /* Camera */
    it = d.FindMember("camera");
    if (it != d.MemberEnd()) {
        m_camera = read_camera(it->value);
    } else {
        // LOG ERROR
        // FAIL
    }

    /* Rendering options */
    it = d.FindMember("options");
    if (it != d.MemberEnd()) {
        m_options = read_params(it->value);
    }
}

sptr<Scene> Scene::create_json(const std::string &path)
{
    sptr<_Scene> scene = std::make_shared<_Scene>();
    scene->init_with_json(path);

    return scene;
}
