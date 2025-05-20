#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>
#include <glm.hpp>
#include <matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Mesh.h"
#include "Shader.h"

class Model {
public:
    std::vector<Mesh> meshes;
    std::vector<glm::mat4> meshTransforms;
    std::string directory;

    Model(std::string const& path) {
        loadModel(path);
        meshTransforms.resize(meshes.size(), glm::mat4(1.0f));
    }

    void Draw(Shader& shader) {
        for (size_t i = 0; i < meshes.size(); i++) {
            shader.setMat4("model", meshTransforms[i]);
            meshes[i].Draw(shader);
        }
    }

    void UpdateTransform(int meshIndex, const glm::mat4& transform) {
        if (meshIndex >= 0 && meshIndex < meshTransforms.size()) {
            meshTransforms[meshIndex] = transform;
        }
    }

private:
    void loadModel(std::string const& path) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path,
            aiProcess_Triangulate |
            aiProcess_GenNormals |
            aiProcess_FlipUVs);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "ASSIMP ERROR: " << importer.GetErrorString() << std::endl;
            return;
        }
        directory = path.substr(0, path.find_last_of('/'));
        processNode(scene->mRootNode, scene);
    }

    void processNode(aiNode* node, const aiScene* scene) {
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;
            vertex.Position = glm::vec3(
                mesh->mVertices[i].x,
                mesh->mVertices[i].y,
                mesh->mVertices[i].z);

            if (mesh->HasNormals()) {
                vertex.Normal = glm::vec3(
                    mesh->mNormals[i].x,
                    mesh->mNormals[i].y,
                    mesh->mNormals[i].z);
            }
            vertices.push_back(vertex);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        return Mesh(vertices, indices);
    }
};

#endif // MODEL_H