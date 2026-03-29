#include "models/TreeMesh.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <cstdio>
#include <vector>

namespace parking {

namespace {

constexpr float kPi = 3.14159265358979323846f;

void pushVertex(std::vector<float>& v, const glm::vec3& p, const glm::vec3& n) {
  v.push_back(p.x);
  v.push_back(p.y);
  v.push_back(p.z);
  v.push_back(n.x);
  v.push_back(n.y);
  v.push_back(n.z);
  v.push_back(0.0f);
  v.push_back(0.0f);
}

/// Open cylinder (no caps), y0 < y1, centered on Y axis.
void buildCylinder(std::vector<float>& verts, std::vector<unsigned int>& idx, unsigned int& baseVertex, float y0,
                   float y1, float r, int segments) {
  const int seg = std::max(3, segments);
  const unsigned int ring0 = baseVertex;
  for (int i = 0; i < seg; ++i) {
    const float t = (kPi * 2.0f * static_cast<float>(i)) / static_cast<float>(seg);
    const float x = std::cos(t) * r;
    const float z = std::sin(t) * r;
    const glm::vec3 n = glm::normalize(glm::vec3(x, 0.0f, z));
    pushVertex(verts, glm::vec3(x, y0, z), n);
  }
  const unsigned int ring1 = baseVertex + static_cast<unsigned int>(seg);
  for (int i = 0; i < seg; ++i) {
    const float t = (kPi * 2.0f * static_cast<float>(i)) / static_cast<float>(seg);
    const float x = std::cos(t) * r;
    const float z = std::sin(t) * r;
    const glm::vec3 n = glm::normalize(glm::vec3(x, 0.0f, z));
    pushVertex(verts, glm::vec3(x, y1, z), n);
  }
  for (int i = 0; i < seg; ++i) {
    const int j = (i + 1) % seg;
    idx.push_back(ring0 + static_cast<unsigned int>(i));
    idx.push_back(ring1 + static_cast<unsigned int>(i));
    idx.push_back(ring0 + static_cast<unsigned int>(j));
    idx.push_back(ring0 + static_cast<unsigned int>(j));
    idx.push_back(ring1 + static_cast<unsigned int>(i));
    idx.push_back(ring1 + static_cast<unsigned int>(j));
  }
  baseVertex += static_cast<unsigned int>(2 * seg);
}

/// Cone: apex at yApex, base ring at yBase (radius rBase). Side normals point outward.
void buildCone(std::vector<float>& verts, std::vector<unsigned int>& idx, unsigned int& baseVertex, float yBase,
               float yApex, float rBase, int segments) {
  const int seg = std::max(3, segments);
  const float h = yApex - yBase;
  const unsigned int apexIdx = baseVertex;
  pushVertex(verts, glm::vec3(0.0f, yApex, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  baseVertex += 1;

  const unsigned int ringStart = baseVertex;
  for (int i = 0; i < seg; ++i) {
    const float t = (kPi * 2.0f * static_cast<float>(i)) / static_cast<float>(seg);
    const float x = std::cos(t) * rBase;
    const float z = std::sin(t) * rBase;
    const glm::vec3 rad = glm::normalize(glm::vec3(x, 0.0f, z));
    const glm::vec3 n = glm::normalize(glm::vec3(rad.x, rBase / std::max(h, 0.01f), rad.z));
    pushVertex(verts, glm::vec3(x, yBase, z), n);
  }
  baseVertex += static_cast<unsigned int>(seg);

  for (int i = 0; i < seg; ++i) {
    const int j = (i + 1) % seg;
    idx.push_back(apexIdx);
    idx.push_back(ringStart + static_cast<unsigned int>(i));
    idx.push_back(ringStart + static_cast<unsigned int>(j));
  }
}

}  // namespace

TreeMesh::~TreeMesh() {
  if (vao_) {
    glDeleteVertexArrays(1, &vao_);
  }
  if (vbo_) {
    glDeleteBuffers(1, &vbo_);
  }
  if (ebo_) {
    glDeleteBuffers(1, &ebo_);
  }
}

void TreeMesh::create() {
  if (vao_) {
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vbo_);
    glDeleteBuffers(1, &ebo_);
    vao_ = vbo_ = ebo_ = 0;
  }

  std::vector<float> verts;
  std::vector<unsigned int> indices;
  unsigned int bv = 0;

  buildCylinder(verts, indices, bv, 0.0f, 1.85f, 0.24f, 12);

  const GLsizei trunkEnd = static_cast<GLsizei>(indices.size());
  trunkIndexCount_ = trunkEnd;

  buildCone(verts, indices, bv, 1.45f, 3.75f, 1.65f, 14);
  buildCone(verts, indices, bv, 3.15f, 5.45f, 1.05f, 12);

  foliageIndexCount_ = static_cast<GLsizei>(indices.size()) - trunkEnd;

  if (verts.empty() || indices.empty()) {
    return;
  }

  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vbo_);
  glGenBuffers(1, &ebo_);

  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(verts.size() * sizeof(float)), verts.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
               indices.data(), GL_STATIC_DRAW);

  const GLsizei stride = 8 * static_cast<GLsizei>(sizeof(float));
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void*>(sizeof(float) * 3));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void*>(sizeof(float) * 6));
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);
  std::printf("TreeMesh: procedural trunk + foliage (%zu tris)\n", indices.size() / 3);
}

void TreeMesh::draw(const glm::mat4& model, const glm::mat4& viewProj, const glm::mat4& lightViewProj,
                    Shader& shader, const glm::vec3& lightDirWorld, const glm::vec3& cameraPos, float ambient,
                    float specularStrength, int shadowMapTextureUnit, int diffuseTextureUnit) const {
  if (!ready()) {
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
  shader.setBool("uUseTexture", false);

  glActiveTexture(GL_TEXTURE0 + diffuseTextureUnit);
  // Caller binds a 1x1 white texture for unused diffuse sampler.

  glBindVertexArray(vao_);

  shader.setVec3("uBaseColor", 0.32f, 0.2f, 0.1f);
  glDrawElements(GL_TRIANGLES, trunkIndexCount_, GL_UNSIGNED_INT, nullptr);

  shader.setVec3("uBaseColor", 0.12f, 0.52f, 0.2f);
  const void* offset = reinterpret_cast<const void*>(static_cast<size_t>(trunkIndexCount_) * sizeof(unsigned int));
  glDrawElements(GL_TRIANGLES, foliageIndexCount_, GL_UNSIGNED_INT, offset);

  glBindVertexArray(0);
  glActiveTexture(GL_TEXTURE0);
}

void TreeMesh::drawShadowCaster(const glm::mat4& model, const glm::mat4& lightViewProj, Shader& depthShader) const {
  if (!ready()) {
    return;
  }
  depthShader.use();
  depthShader.setMat4("uModel", glm::value_ptr(model));
  depthShader.setMat4("uLightViewProj", glm::value_ptr(lightViewProj));
  glBindVertexArray(vao_);
  glDrawElements(GL_TRIANGLES, trunkIndexCount_, GL_UNSIGNED_INT, nullptr);
  const void* offset = reinterpret_cast<const void*>(static_cast<size_t>(trunkIndexCount_) * sizeof(unsigned int));
  glDrawElements(GL_TRIANGLES, foliageIndexCount_, GL_UNSIGNED_INT, offset);
  glBindVertexArray(0);
}

}  // namespace parking
