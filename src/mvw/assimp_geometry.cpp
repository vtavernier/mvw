#include <epoxy/gl.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mvw/assimp_geometry.hpp"

assimp_geometry::assimp_geometry(const std::string &geometry_path)
    : mvw_geometry()
{
    Assimp::Importer importer;

    const aiScene * scene = importer.ReadFile(geometry_path,
                                              aiProcess_Triangulate |
                                              aiProcess_JoinIdenticalVertices |
                                              aiProcess_GenSmoothNormals |
                                              aiProcess_ImproveCacheLocality |
                                              aiProcess_SortByPType);

    if (!scene) {
        throw std::runtime_error(importer.GetErrorString());
    }

    if (scene->mNumMeshes == 0) {
        throw std::runtime_error("No meshes found in imported file");
    }

    glm::vec3 d_min(0., 0., 0.),
        d_max(0., 0., 0.);
    glm::dvec3 d_centroid(0., 0., 0.);
    size_t centroid_count = 0;

    // Just get the first mesh
    for (size_t mi = 0; mi < scene->mNumMeshes; ++mi) {
        aiMesh * mesh = scene->mMeshes[mi];

        std::vector<float> vertices;
        std::vector<uint32_t> indices;

        vertices.reserve(mesh->mNumVertices * 8);
        indices.reserve(mesh->mNumFaces * 3);

        for (size_t i = 0; i < mesh->mNumVertices; ++i) {
            glm::vec3 p(mesh->mVertices[i].x,
                        mesh->mVertices[i].y,
                        mesh->mVertices[i].z);

            if (p.x < d_min.x) d_min.x = p.x;
            if (p.y < d_min.y) d_min.y = p.y;
            if (p.z < d_min.z) d_min.z = p.z;

            if (p.x > d_max.x) d_max.x = p.x;
            if (p.x > d_max.y) d_max.y = p.y;
            if (p.z > d_max.z) d_max.z = p.z;

            d_centroid += p;

            vertices.push_back(p.x);
            vertices.push_back(p.y);
            vertices.push_back(p.z);

            vertices.push_back(mesh->mNormals[i].x);
            vertices.push_back(mesh->mNormals[i].y);
            vertices.push_back(mesh->mNormals[i].z);

            if (mesh->mNumUVComponents[0] >= 2) {
                vertices.push_back(mesh->mTextureCoords[0][i].x);
                vertices.push_back(mesh->mTextureCoords[0][i].y);
            } else {
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
            }
        }

        centroid_count += mesh->mNumVertices;

        for (size_t i = 0; i < mesh->mNumFaces; ++i) {
            auto face = mesh->mFaces[i];

            if (face.mNumIndices < 3) break;

            std::copy(face.mIndices, face.mIndices + face.mNumIndices,
                      std::back_inserter(indices));
        }

        add_vertex_data(vertices, indices);
    }

    bbox_min_ = d_min;
    bbox_max_ = d_max;
    if (centroid_count > 0)
        bbox_centroid_ = d_centroid / static_cast<double>(centroid_count);
}