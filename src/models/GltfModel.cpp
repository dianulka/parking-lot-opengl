#include "models/GltfModel.hpp"

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

namespace parking {

namespace {

glm::mat4 mat4_assimp(const aiMatrix4x4& m) {
  return glm::transpose(glm::make_mat4(&m.a1));
}

GLuint makeWhiteFallback() {
  GLuint t = 0;
  glGenTextures(1, &t);
  const unsigned char px[] = {255, 255, 255, 255};
  glBindTexture(GL_TEXTURE_2D, t);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glBindTexture(GL_TEXTURE_2D, 0);
  return t;
}

GLuint loadTextureFromFile(const std::string& path) {
  int w = 0;
  int h = 0;
  int n = 0;
  unsigned char* data = stbi_load(path.c_str(), &w, &h, &n, 4);
  if (!data || w <= 0 || h <= 0) {
    if (data) {
      stbi_image_free(data);
    }
    std::fprintf(stderr, "GltfModel: nie wczytano tekstury: %s\n", path.c_str());
    return 0;
  }

  GLuint tex = 0;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glBindTexture(GL_TEXTURE_2D, 0);
  stbi_image_free(data);
  return tex;
}

GLuint loadTextureEmbedded(const aiTexture* tex) {
  if (!tex) {
    return 0;
  }
  GLuint glt = 0;
  glGenTextures(1, &glt);
  glBindTexture(GL_TEXTURE_2D, glt);
  if (tex->mHeight == 0) {
    int w = 0;
    int h = 0;
    int n = 0;
    const auto* bytes = reinterpret_cast<const unsigned char*>(tex->pcData);
    unsigned char* data = stbi_load_from_memory(bytes, static_cast<int>(tex->mWidth), &w, &h, &n, 4);
    if (!data || w <= 0 || h <= 0) {
      if (data) {
        stbi_image_free(data);
      }
      glDeleteTextures(1, &glt);
      glBindTexture(GL_TEXTURE_2D, 0);
      return 0;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
  } else {
    const int w = static_cast<int>(tex->mWidth);
    const int h = static_cast<int>(tex->mHeight);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                   reinterpret_cast<const unsigned char*>(tex->pcData));
  }
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glBindTexture(GL_TEXTURE_2D, 0);
  return glt;
}

GLuint loadMaterialDiffuse(const aiMaterial* mat, const aiScene* scene, const std::string& dir, glm::vec3& baseColor,
                           bool& useTexture) {
  useTexture = false;
  baseColor = glm::vec3(0.85f);
  aiColor3D c{};
  if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, c) == AI_SUCCESS) {
    baseColor = glm::vec3(c.r, c.g, c.b);
  }

  aiString str{};
  if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &str) != AI_SUCCESS) {
    if (mat->GetTexture(aiTextureType_BASE_COLOR, 0, &str) != AI_SUCCESS) {
      return 0;
    }
  }

  const char* p = str.C_Str();
  if (p[0] == '*') {
    const int idx = std::atoi(p + 1);
    if (idx >= 0 && scene && static_cast<unsigned>(idx) < scene->mNumTextures) {
      const GLuint t = loadTextureEmbedded(scene->mTextures[idx]);
      if (t != 0) {
        useTexture = true;
        return t;
      }
    }
    return 0;
  }

  std::filesystem::path rel(p);
  std::filesystem::path full = std::filesystem::path(dir) / rel;
  const GLuint t = loadTextureFromFile(full.string());
  if (t != 0) {
    useTexture = true;
  }
  return t;
}

void processMesh(const aiMesh* mesh, const aiScene* scene, const glm::mat4& transform, const std::string& dir,
                 std::vector<GltfModel::Part>& out) {
  if (!mesh || mesh->mNumVertices == 0 || mesh->mNumFaces == 0) {
    return;
  }

  const glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(transform)));

  std::vector<float> verts;
  verts.reserve(static_cast<size_t>(mesh->mNumVertices) * 8u);
  for (unsigned v = 0; v < mesh->mNumVertices; ++v) {
    glm::vec3 p(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z);
    glm::vec4 pw = transform * glm::vec4(p, 1.0f);
    glm::vec3 n(0.0f, 1.0f, 0.0f);
    if (mesh->HasNormals()) {
      n = glm::vec3(mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z);
      n = glm::normalize(normalMat * n);
    }
    float u = 0.0f;
    float vt = 0.0f;
    if (mesh->mTextureCoords[0]) {
      u = mesh->mTextureCoords[0][v].x;
      vt = mesh->mTextureCoords[0][v].y;
    }
    verts.push_back(pw.x);
    verts.push_back(pw.y);
    verts.push_back(pw.z);
    verts.push_back(n.x);
    verts.push_back(n.y);
    verts.push_back(n.z);
    verts.push_back(u);
    verts.push_back(vt);
  }

  std::vector<unsigned int> indices;
  indices.reserve(static_cast<size_t>(mesh->mNumFaces) * 3u);
  for (unsigned i = 0; i < mesh->mNumFaces; ++i) {
    const aiFace& f = mesh->mFaces[i];
    for (unsigned j = 0; j < f.mNumIndices; ++j) {
      indices.push_back(f.mIndices[j]);
    }
  }

  glm::vec3 baseColor(0.85f);
  bool useTex = false;
  GLuint tex = 0;
  if (mesh->mMaterialIndex < scene->mNumMaterials) {
    const aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
    tex = loadMaterialDiffuse(mat, scene, dir, baseColor, useTex);
  }

  GltfModel::Part part{};
  part.indexCount = static_cast<GLsizei>(indices.size());
  part.baseColor = baseColor;
  part.useTexture = useTex;
  part.diffuseTex = tex;

  glGenVertexArrays(1, &part.vao);
  glGenBuffers(1, &part.vbo);
  glGenBuffers(1, &part.ebo);

  glBindVertexArray(part.vao);
  glBindBuffer(GL_ARRAY_BUFFER, part.vbo);
  glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(verts.size() * sizeof(float)), verts.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, part.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)), indices.data(),
               GL_STATIC_DRAW);

  const GLsizei stride = 8 * static_cast<GLsizei>(sizeof(float));
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void*>(sizeof(float) * 3));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void*>(sizeof(float) * 6));
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  out.push_back(part);
}

void accumulateWorldBounds(aiNode* node, const aiScene* scene, const glm::mat4& parent, glm::vec3& mn,
                           glm::vec3& mx) {
  if (!node || !scene) {
    return;
  }
  const glm::mat4 transform = parent * mat4_assimp(node->mTransformation);
  for (unsigned i = 0; i < node->mNumMeshes; ++i) {
    const unsigned mid = node->mMeshes[i];
    if (mid >= scene->mNumMeshes) {
      continue;
    }
    const aiMesh* mesh = scene->mMeshes[mid];
    for (unsigned v = 0; v < mesh->mNumVertices; ++v) {
      const glm::vec4 p = transform * glm::vec4(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z, 1.0f);
      mn = glm::min(mn, glm::vec3(p));
      mx = glm::max(mx, glm::vec3(p));
    }
  }
  for (unsigned i = 0; i < node->mNumChildren; ++i) {
    accumulateWorldBounds(node->mChildren[i], scene, transform, mn, mx);
  }
}

void processNode(aiNode* node, const aiScene* scene, const glm::mat4& parent, const std::string& dir,
                 std::vector<GltfModel::Part>& out) {
  if (!node || !scene) {
    return;
  }
  const glm::mat4 transform = parent * mat4_assimp(node->mTransformation);
  for (unsigned i = 0; i < node->mNumMeshes; ++i) {
    const unsigned mid = node->mMeshes[i];
    if (mid < scene->mNumMeshes) {
      processMesh(scene->mMeshes[mid], scene, transform, dir, out);
    }
  }
  for (unsigned i = 0; i < node->mNumChildren; ++i) {
    processNode(node->mChildren[i], scene, transform, dir, out);
  }
}

}  // namespace

GltfModel::~GltfModel() {
  for (Part& p : parts_) {
    if (p.vao) {
      glDeleteVertexArrays(1, &p.vao);
      p.vao = 0;
    }
    if (p.vbo) {
      glDeleteBuffers(1, &p.vbo);
      p.vbo = 0;
    }
    if (p.ebo) {
      glDeleteBuffers(1, &p.ebo);
      p.ebo = 0;
    }
    if (p.diffuseTex) {
      glDeleteTextures(1, &p.diffuseTex);
      p.diffuseTex = 0;
    }
  }
  parts_.clear();
}

bool GltfModel::loadFromFile(const std::string& path, float normalizeMaxExtentMeters) {
  for (Part& p : parts_) {
    if (p.vao) {
      glDeleteVertexArrays(1, &p.vao);
    }
    if (p.vbo) {
      glDeleteBuffers(1, &p.vbo);
    }
    if (p.ebo) {
      glDeleteBuffers(1, &p.ebo);
    }
    if (p.diffuseTex) {
      glDeleteTextures(1, &p.diffuseTex);
    }
  }
  parts_.clear();

  Assimp::Importer importer;
  constexpr unsigned flags = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals |
                             aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality;
  const aiScene* scene = importer.ReadFile(path, flags);
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    std::fprintf(stderr, "GltfModel: Assimp: %s\n", importer.GetErrorString());
    return false;
  }

  const std::string dir = std::filesystem::path(path).parent_path().string();

  glm::mat4 root(1.0f);
  if (normalizeMaxExtentMeters > 0.0f) {
    glm::vec3 mn(1e30f);
    glm::vec3 mx(-1e30f);
    accumulateWorldBounds(scene->mRootNode, scene, glm::mat4(1.0f), mn, mx);
    const glm::vec3 ext = mx - mn;
    const float maxDim = std::max(std::max(ext.x, ext.y), ext.z);
    if (maxDim > 1e-6f) {
      const float s = normalizeMaxExtentMeters / maxDim;
      const glm::vec3 ctr((mn.x + mx.x) * 0.5f, mn.y, (mn.z + mx.z) * 0.5f);
      root = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -mn.y * s, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(s))
             * glm::translate(glm::mat4(1.0f), -ctr);
    }
  }

  processNode(scene->mRootNode, scene, root, dir, parts_);

  if (parts_.empty()) {
    std::fprintf(stderr, "GltfModel: brak siatki: %s\n", path.c_str());
    return false;
  }

  std::printf("GltfModel: %s (%zu czesci)\n", path.c_str(), parts_.size());
  return true;
}

void GltfModel::draw(const glm::mat4& model, const glm::mat4& viewProj, const glm::mat4& lightViewProj, Shader& shader,
                     const glm::vec3& lightDirWorld, const glm::vec3& cameraPos, float ambient, float specularStrength,
                     int shadowMapTextureUnit, int diffuseTextureUnit, GLuint whiteTexture) const {
  if (parts_.empty()) {
    return;
  }

  const glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(model)));
  shader.use();
  shader.setMat4("uModel", glm::value_ptr(model));
  shader.setMat4("uViewProj", glm::value_ptr(viewProj));
  shader.setMat4("uLightViewProj", glm::value_ptr(lightViewProj));
  shader.setMat3("uNormalMat", glm::value_ptr(normalMat));
  shader.setVec3("uLightDir", lightDirWorld.x, lightDirWorld.y, lightDirWorld.z);
  shader.setVec3("uCameraPos", cameraPos.x, cameraPos.y, cameraPos.z);
  shader.setFloat("uAmbient", ambient);
  shader.setFloat("uSpecularStrength", specularStrength);
  shader.setInt("uShadowMap", shadowMapTextureUnit);
  shader.setInt("uDiffuse", diffuseTextureUnit);

  for (const Part& p : parts_) {
    shader.setBool("uUseTexture", p.useTexture);
    shader.setVec3("uBaseColor", p.baseColor.x, p.baseColor.y, p.baseColor.z);
    glActiveTexture(GL_TEXTURE0 + diffuseTextureUnit);
    glBindTexture(GL_TEXTURE_2D, p.useTexture && p.diffuseTex ? p.diffuseTex : whiteTexture);
    glBindVertexArray(p.vao);
    glDrawElements(GL_TRIANGLES, p.indexCount, GL_UNSIGNED_INT, nullptr);
  }

  glBindVertexArray(0);
  glActiveTexture(GL_TEXTURE0);
}

void GltfModel::drawShadow(const glm::mat4& model, const glm::mat4& lightViewProj, Shader& depthShader) const {
  if (parts_.empty()) {
    return;
  }
  depthShader.use();
  depthShader.setMat4("uModel", glm::value_ptr(model));
  depthShader.setMat4("uLightViewProj", glm::value_ptr(lightViewProj));
  for (const Part& p : parts_) {
    glBindVertexArray(p.vao);
    glDrawElements(GL_TRIANGLES, p.indexCount, GL_UNSIGNED_INT, nullptr);
  }
  glBindVertexArray(0);
}

}  // namespace parking
