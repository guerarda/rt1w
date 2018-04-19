#include "primitive.hpp"
#include "mesh.hpp"
#include "material.hpp"
#include "bvh.hpp"
#include "value.hpp"

#include <unordered_map>
#include <functional>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader/tiny_obj_loader.h"

/* Hash & Equal fucntion for index_t so they can be put in an unordered_map */
namespace std {
    /* See https://stackoverflow.com/questions/4948780/magic-number-in-boosthash-combine */
    template <>
    struct hash<tinyobj::index_t> {
        std::size_t operator()(const tinyobj::index_t &ix) const noexcept {
            std::size_t seed = 0;
            seed ^= (size_t)ix.vertex_index + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= (size_t)ix.normal_index + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= (size_t)ix.texcoord_index + 0x9e3779b9 + (seed << 6) + (seed >> 2);

            return seed;
        }
    };
    template<>
    struct equal_to<tinyobj::index_t> {
        std::size_t operator()(const tinyobj::index_t &lhs,
                               const tinyobj::index_t &rhs) const noexcept {
            return lhs.vertex_index == rhs.vertex_index
                && lhs.normal_index == rhs.normal_index
                && lhs.texcoord_index == rhs.texcoord_index;
        }
    };
}

sptr<Primitive> Primitive::load_obj(const std::string &path)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> obj_shapes;
    std::vector<tinyobj::material_t> obj_materials;

    std::string err;
    bool ok = tinyobj::LoadObj(&attrib,
                               &obj_shapes,
                               &obj_materials,
                               &err,
                               path.c_str());
    ERROR_IF(!err.empty(), "load_obj: %s", err.c_str());
    if (!ok) {
        return nullptr;
    }

    /* Vector for storing vertex data*/
    std::vector<v3f> vertices;
    std::vector<v3f> normals;
    std::vector<v2f> texcoords;

    /* Map that contains the translation from index_t to an index in our
     * vertex data */
    std::unordered_map<tinyobj::index_t, uint32_t> remap;

    /* For each mesh, store a value that contains the mesh indices */
    std::vector<sptr<Value>> mesh_indices;

    for (auto &s : obj_shapes) {

        std::vector<uint32_t> indices;

        for (auto &idx : s.mesh.indices) {
            uint32_t i;
            auto it = remap.find(idx);

            if (it != remap.end()) {
                i = it->second;
            }
            else {
                /* 'index' represent a vertex we haven't seen yet, had it to
                 * the vertex data and create a remaped index for it */
                const float *vp = (const float *)(&attrib.vertices[3 * (size_t)idx.vertex_index]);
                const float *np = (const float *)(&attrib.vertices[3 * (size_t)idx.normal_index]);
                const float *tp = (const float *)(&attrib.vertices[3 * (size_t)idx.texcoord_index]);

                vertices.push_back({ vp[0], vp[1], vp[2] });
                normals.push_back({ np[0], np[1], np[2] });
                texcoords.push_back({ tp[0], tp[1] });

                i = (uint32_t)remap.size();
                remap.insert({ idx, i });
            }
            indices.push_back(i);
        }
        mesh_indices.push_back(Value::create(TYPE_UINT32,
                                             indices.data(),
                                             indices.size()));
    }

    /* Now that we went through all the indices, create the VertexData struct */
    size_t nv = vertices.size();
    uptr<v3f[]> v = std::make_unique<v3f[]>(vertices.size());
    std::copy(vertices.begin(), vertices.end(), v.get());

    uptr<v3f[]> n = std::make_unique<v3f[]>(normals.size());
    std::copy(normals.begin(), normals.end(), n.get());

    uptr<v2f[]> uv = std::make_unique<v2f[]>(texcoords.size());
    std::copy(texcoords.begin(), texcoords.end(), uv.get());

    sptr<VertexData> vd = VertexData::create(nv, v, n, uv);

    /* Loop over the Values containing the indices for each mesh and create a Mesh
     * with each and the VertexData */
    std::vector<sptr<Primitive>> primitives;
    std::vector<sptr<Mesh>> meshes;

    for (auto &indices_value : mesh_indices) {
        size_t nt = indices_value->count() / 3;
        sptr<Mesh> m = Mesh::create(nt, vd, indices_value);
        const std::vector<sptr<Shape>> shapes = m->faces();

        for (auto &shp : shapes) {
            primitives.push_back(Primitive::create(shp,
                                                   Lambertian::create(Texture::create_color({ .5f, .5f, .5f }))));

        }
    }

    return BVHNode::create(primitives);


}