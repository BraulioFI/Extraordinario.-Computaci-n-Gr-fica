#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "SOIL2/SOIL2.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "AnimationStructs.h"   // BoneInfo (una sola definicion)
#include "AssimpGLMHelpers.h"
#include "Mesh.h"
#include "Shader.h"

using namespace std;

GLint TextureFromFile(const char* path, string directory);
GLint TextureFromEmbedded(const aiTexture* tex);

class Model
{
public:
    bool usarKd = true;   // false = ignorar el color del .mtl y usar el cafe por defecto
    Model(GLchar* path, bool usarColorMtl = true) { usarKd = usarColorMtl; this->loadModel(path); }

    glm::vec3 bbMin = glm::vec3(1e9f), bbMax = glm::vec3(-1e9f);   // caja que envuelve al modelo

    void Draw(Shader shader) {
        for (GLuint i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }

    int MeshCount() { return (int)meshes.size(); }
    void DrawMesh(int i, Shader shader) {
        if (i >= 0 && i < (int)meshes.size()) meshes[i].Draw(shader);
    }

    // Accesores para Animation::ReadMissingBones
    std::map<std::string, BoneInfo>& GetBoneInfoMap() { return m_BoneInfoMap; }
    int& GetBoneCount() { return m_BoneCounter; }

private:
    vector<Mesh>    meshes;
    string          directory;
    vector<Texture> textures_loaded;

    std::map<std::string, BoneInfo> m_BoneInfoMap;
    int m_BoneCounter = 0;

    void SetVertexBoneDataToDefault(Vertex& v) {
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
            v.m_BoneIDs[i] = -1;
            v.m_Weights[i] = 0.0f;
        }
    }

    void SetVertexBoneData(Vertex& v, int boneID, float weight) {
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
            if (v.m_BoneIDs[i] < 0) {
                v.m_BoneIDs[i] = boneID;
                v.m_Weights[i] = weight;
                break;
            }
        }
    }

    void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene*)
    {
        for (int b = 0; b < (int)mesh->mNumBones; b++) {
            int         boneID = -1;
            std::string boneName = mesh->mBones[b]->mName.C_Str();

            if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end()) {
                BoneInfo newBone;
                newBone.id = m_BoneCounter;
                newBone.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(
                    mesh->mBones[b]->mOffsetMatrix);
                m_BoneInfoMap[boneName] = newBone;
                boneID = m_BoneCounter++;
            }
            else {
                boneID = m_BoneInfoMap[boneName].id;
            }

            auto* weights = mesh->mBones[b]->mWeights;
            int   numWeights = mesh->mBones[b]->mNumWeights;
            for (int w = 0; w < numWeights; w++) {
                int   vId = weights[w].mVertexId;
                float weight = weights[w].mWeight;
                if (vId < (int)vertices.size())
                    SetVertexBoneData(vertices[vId], boneID, weight);
            }
        }
    }

    void loadModel(string path) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);
        if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        directory = path.substr(0, path.find_last_of('/'));
        processNode(scene->mRootNode, scene);
    }

    void processNode(aiNode* node, const aiScene* scene) {
        for (GLuint i = 0; i < node->mNumMeshes; i++)
            meshes.push_back(processMesh(scene->mMeshes[node->mMeshes[i]], scene));
        for (GLuint i = 0; i < node->mNumChildren; i++)
            processNode(node->mChildren[i], scene);
    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
        vector<Vertex>  vertices;
        vector<GLuint>  indices;
        vector<Texture> textures;

        for (GLuint i = 0; i < mesh->mNumVertices; i++) {
            Vertex v;
            SetVertexBoneDataToDefault(v);

            v.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };

            bbMin = glm::min(bbMin, v.Position);
            bbMax = glm::max(bbMax, v.Position);

            if (mesh->HasNormals())
                v.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
            else
                v.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
            if (mesh->mTextureCoords[0])
                v.TexCoords = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
            else
                v.TexCoords = glm::vec2(0.0f);

            vertices.push_back(v);
        }

        for (GLuint i = 0; i < mesh->mNumFaces; i++) {
            aiFace& face = mesh->mFaces[i];
            for (GLuint j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        if (mesh->mMaterialIndex >= 0) {
            aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
            auto diffuse = loadMaterialTextures(mat, aiTextureType_DIFFUSE, "texture_diffuse", scene);
            auto specular = loadMaterialTextures(mat, aiTextureType_SPECULAR, "texture_specular", scene);
            textures.insert(textures.end(), diffuse.begin(), diffuse.end());
            textures.insert(textures.end(), specular.begin(), specular.end());
        }

        // Si la malla no trae textura, se usa un color solido por defecto
        if (textures.empty()) {
            Texture t;
            GLuint id;
            glGenTextures(1, &id);
            unsigned char px[3] = { 140, 90, 50 };   // cafe por defecto (R, G, B)

            // Si el material trae su propio color (Kd del .mtl), se usa ese en lugar del cafe.
            // Con usarKd = false se ignora y se queda el cafe (se pide al crear el Model).
            if (usarKd && mesh->mMaterialIndex >= 0) {
                aiMaterial* mc = scene->mMaterials[mesh->mMaterialIndex];
                aiString nombreMat;
                mc->Get(AI_MATKEY_NAME, nombreMat);
                aiColor3D kd(0.0f, 0.0f, 0.0f);
                if (string(nombreMat.C_Str()) != "DefaultMaterial" &&
                    mc->Get(AI_MATKEY_COLOR_DIFFUSE, kd) == AI_SUCCESS) {
                    px[0] = (unsigned char)(glm::clamp(kd.r, 0.0f, 1.0f) * 255.0f);
                    px[1] = (unsigned char)(glm::clamp(kd.g, 0.0f, 1.0f) * 255.0f);
                    px[2] = (unsigned char)(glm::clamp(kd.b, 0.0f, 1.0f) * 255.0f);
                }
            }

            glBindTexture(GL_TEXTURE_2D, id);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, px);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glBindTexture(GL_TEXTURE_2D, 0);
            t.id = id;
            t.type = "texture_diffuse";
            t.path.Set("color_por_defecto");
            textures.push_back(t);
        }

        ExtractBoneWeightForVertices(vertices, mesh, scene);
        return Mesh(vertices, indices, textures);
    }

    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName, const aiScene* scene) {
        vector<Texture> textures;
        for (GLuint i = 0; i < mat->GetTextureCount(type); i++) {
            aiString str;
            mat->GetTexture(type, i, &str);
            bool skip = false;
            for (auto& t : textures_loaded) {
                if (t.path == str) { textures.push_back(t); skip = true; break; }
            }
            if (!skip) {
                Texture texture;

                // 1) Textura embebida dentro del FBX (caso de Mixamo)
                const aiTexture* embedded = nullptr;
                if (str.C_Str()[0] == '*') {
                    unsigned int idx = (unsigned int)atoi(str.C_Str() + 1);
                    if (idx < scene->mNumTextures) embedded = scene->mTextures[idx];
                }
                if (embedded) {
                    texture.id = TextureFromEmbedded(embedded);
                }
                else {
                    // 2) Textura como archivo: primero la ruta tal cual (relativa al modelo);
                    //    si no existe, se prueba solo con el nombre del archivo.
                    string p = str.C_Str();
                    for (auto& c : p) if (c == '\\') c = '/';
                    string full = directory + '/' + p;
                    ifstream f(full.c_str());
                    if (!f.good()) {
                        size_t pos = p.find_last_of('/');
                        if (pos != string::npos) p = p.substr(pos + 1);
                    }
                    texture.id = TextureFromFile(p.c_str(), directory);
                }

                texture.type = typeName;
                texture.path = str;
                textures.push_back(texture);
                textures_loaded.push_back(texture);
            }
        }
        return textures;
    }
};

GLint TextureFromFile(const char* path, string directory)
{
    string filename = directory + '/' + string(path);
    GLuint textureID;
    glGenTextures(1, &textureID);
    int width, height;
    unsigned char* image = SOIL_load_image(filename.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
    if (!image) {
        cout << "ERROR::TEXTURE: " << filename << endl;
        cout << "SOIL: " << SOIL_last_result() << endl;
        return 0;
    }
    glBindTexture(GL_TEXTURE_2D, textureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0);
    return textureID;
}

// Carga una textura que viene embebida dentro del modelo (por ejemplo, FBX de Mixamo)
GLint TextureFromEmbedded(const aiTexture* tex)
{
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width = 0, height = 0, channels = 0;
    unsigned char* image = nullptr;
    vector<unsigned char> raw;
    bool fromSOIL = false;

    if (tex->mHeight == 0) {
        // Imagen comprimida (PNG/JPG): mWidth es el tamano en bytes
        image = SOIL_load_image_from_memory(
            reinterpret_cast<const unsigned char*>(tex->pcData),
            (int)tex->mWidth, &width, &height, &channels, SOIL_LOAD_RGB);
        fromSOIL = true;
    }
    else {
        // Pixeles sin comprimir
        width = (int)tex->mWidth;
        height = (int)tex->mHeight;
        raw.resize(width * height * 3);
        for (int i = 0; i < width * height; i++) {
            raw[i * 3 + 0] = tex->pcData[i].r;
            raw[i * 3 + 1] = tex->pcData[i].g;
            raw[i * 3 + 2] = tex->pcData[i].b;
        }
        image = raw.data();
    }

    if (!image) {
        cout << "ERROR::TEXTURE EMBEBIDA" << endl;
        cout << "SOIL: " << SOIL_last_result() << endl;
        return 0;
    }

    glBindTexture(GL_TEXTURE_2D, textureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (fromSOIL) SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0);
    return textureID;
}