#include "models/GrassBlades.hpp"

#include "scene/ParkingGenerator.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <random>
#include <vector>

namespace parking {

namespace {

bool isGrassZone(float x, float z, float halfL, float halfW, float halfGrassL, float halfRoadW) {
  if (std::abs(x) <= halfL && std::abs(z) <= halfW) {
    return false;
  }
  if (std::abs(x) <= halfGrassL && std::abs(z) <= halfRoadW) {
    return false;
  }
  return true;
}

unsigned packCacheKey(const ParkingGenerator& gen) {
  const unsigned a = static_cast<unsigned>(gen.length() * 1000.0f);
  const unsigned b = static_cast<unsigned>(gen.spotCount()) * 2654435761u;
  constexpr unsigned kPlacementVer = 5u;
  return a ^ b ^ (kPlacementVer * 1315423911u);
}

}  // namespace

GrassBlades::~GrassBlades() {
  if (vao_) {
    glDeleteVertexArrays(1, &vao_);
  }
  if (vbo_) {
    glDeleteBuffers(1, &vbo_);
  }
  if (ebo_) {
    glDeleteBuffers(1, &ebo_);
  }
  if (instanceVbo_) {
    glDeleteBuffers(1, &instanceVbo_);
  }
}

void GrassBlades::create() {
  if (vao_) {
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vbo_);
    glDeleteBuffers(1, &ebo_);
    glDeleteBuffers(1, &instanceVbo_);
    vao_ = vbo_ = ebo_ = instanceVbo_ = 0;
  }

  constexpr float kClumpHeightM = 1.25f;
  constexpr float hw = 0.25f;
  constexpr float h = kClumpHeightM;

  const float verts[] = {
      // quad w płaszczyźnie X–Y, normal (0,0,1)
      -hw, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,  //
      hw,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f,  //
      hw,  h,    0.0f, 0.0f, 0.0f, 1.0f,  //
      -hw, h,    0.0f, 0.0f, 0.0f, 1.0f,  //
      // quad w płaszczyźnie Z–Y, normal (1,0,0)
      0.0f, 0.0f, -hw, 1.0f, 0.0f, 0.0f,  //
      0.0f, 0.0f, hw,  1.0f, 0.0f, 0.0f,  //
      0.0f, h,    hw,  1.0f, 0.0f, 0.0f,  //
      0.0f, h,    -hw, 1.0f, 0.0f, 0.0f,  //
  };

  const unsigned int idx[] = {
      0, 1, 2, 2, 3, 0,  //
      4, 5, 6, 6, 7, 4,  //
  };

  indexCount_ = static_cast<GLsizei>(sizeof(idx) / sizeof(idx[0]));

  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vbo_);
  glGenBuffers(1, &ebo_);
  glGenBuffers(1, &instanceVbo_);

  glBindVertexArray(vao_);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

  const GLsizei stride = 6 * static_cast<GLsizei>(sizeof(float));
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void*>(sizeof(float) * 3));
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, instanceVbo_);
  glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4, nullptr);
  glEnableVertexAttribArray(3);
  glVertexAttribDivisor(3, 1);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GrassBlades::rebuildIfNeeded(const ParkingGenerator& gen) {
  const unsigned key = packCacheKey(gen);
  if (key == cacheKey_ && instanceCount_ > 0) {
    return;
  }
  cacheKey_ = key;

  const float L = gen.length();
  const float W = ParkingGenerator::fixedWidth();
  const float aisle = ParkingGenerator::aisleWidthMeters();
  const float mg = ParkingGenerator::grassMarginMeters();
  const float grassL = L + 2.0f * mg;
  const float grassW = W + 2.0f * mg;
  const float roadW = std::max(aisle * 1.12f, 9.5f);

  const float halfL = L * 0.5f;
  const float halfW = W * 0.5f;
  const float halfGrassL = grassL * 0.5f;
  const float halfGrassW = grassW * 0.5f;
  const float halfRoadW = roadW * 0.5f;

  constexpr int kMaxInstances = 240000;
  static_assert(5 * 8 == 40, "tuft layout");
  constexpr float kTuftCenterStep = 52.0f / 3.0f;
  constexpr float kWithinTuftStep = 1.25f / 3.0f;
  constexpr float kJitterTiny = 0.12f / 3.0f;

  std::vector<glm::vec4> inst;
  inst.reserve(static_cast<size_t>(kMaxInstances));

  std::mt19937 rng(static_cast<unsigned>(cacheKey_ ^ 0x9E3779B9u));
  std::uniform_real_distribution<float> jitter(-kJitterTiny, kJitterTiny);
  std::uniform_real_distribution<float> rot01(0.0f, 6.2831853f);
  std::uniform_real_distribution<float> scaleR(0.92f, 1.08f);

  for (float cx = -halfGrassL + kTuftCenterStep * 0.5f; cx < halfGrassL && static_cast<int>(inst.size()) < kMaxInstances;
       cx += kTuftCenterStep) {
    for (float cz = -halfGrassW + kTuftCenterStep * 0.5f; cz < halfGrassW && static_cast<int>(inst.size()) < kMaxInstances;
         cz += kTuftCenterStep) {
      if (!isGrassZone(cx, cz, halfL, halfW, halfGrassL, halfRoadW)) {
        continue;
      }
      for (int r = 0; r < 5 && static_cast<int>(inst.size()) < kMaxInstances; ++r) {
        for (int c = 0; c < 8 && static_cast<int>(inst.size()) < kMaxInstances; ++c) {
          const float ox = (static_cast<float>(c) - 3.5f) * kWithinTuftStep;
          const float oz = (static_cast<float>(r) - 2.0f) * kWithinTuftStep;
          const float jx = cx + ox + jitter(rng);
          const float jz = cz + oz + jitter(rng);
          if (!isGrassZone(jx, jz, halfL, halfW, halfGrassL, halfRoadW)) {
            continue;
          }
          inst.emplace_back(jx, jz, rot01(rng), scaleR(rng));
        }
      }
    }
  }

  instanceCount_ = static_cast<GLsizei>(inst.size());
  if (instanceCount_ <= 0) {
    return;
  }

  glBindBuffer(GL_ARRAY_BUFFER, instanceVbo_);
  glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(inst.size() * sizeof(glm::vec4)), inst.data(),
               GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

namespace {
constexpr float kWindFreq = 2.35f;
constexpr float kWindAmpM = 0.25f;
}  // namespace

void GrassBlades::draw(const glm::mat4& viewProj, const glm::mat4& lightViewProj, Shader& shader,
                       const glm::vec3& lightDir, float ambient, int shadowMapTextureUnit, float timeSec) const {
  if (!ready()) {
    return;
  }
  shader.use();
  shader.setMat4("uViewProj", glm::value_ptr(viewProj));
  shader.setMat4("uLightViewProj", glm::value_ptr(lightViewProj));
  shader.setVec3("uLightDir", lightDir.x, lightDir.y, lightDir.z);
  shader.setFloat("uAmbient", ambient);
  shader.setInt("uShadowMap", shadowMapTextureUnit);
  shader.setFloat("uTime", timeSec);
  shader.setFloat("uWindFreq", kWindFreq);
  shader.setFloat("uWindAmp", kWindAmpM);

  glBindVertexArray(vao_);
  glDrawElementsInstanced(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, nullptr, instanceCount_);
  glBindVertexArray(0);
}

void GrassBlades::drawShadow(const glm::mat4& lightViewProj, Shader& depthShader, float timeSec) const {
  if (!ready()) {
    return;
  }
  depthShader.use();
  depthShader.setMat4("uLightViewProj", glm::value_ptr(lightViewProj));
  depthShader.setFloat("uTime", timeSec);
  depthShader.setFloat("uWindFreq", kWindFreq);
  depthShader.setFloat("uWindAmp", kWindAmpM);
  glBindVertexArray(vao_);
  glDrawElementsInstanced(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, nullptr, instanceCount_);
  glBindVertexArray(0);
}

}  // namespace parking
