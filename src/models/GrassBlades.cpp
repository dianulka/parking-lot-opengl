#include "models/GrassBlades.hpp"

#include "scene/ParkingGenerator.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cmath>
#include <random>
#include <utility>
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
  constexpr unsigned kPlacementVer = 8u;
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

  constexpr float kClumpHeightM = 1.10f;
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
  constexpr float kTwoPi = 6.2831853f;
  /// Zagęszczenie kotwic kępek (mniejszy krok ⇒ więcej kup nie w jednej linii).
  constexpr float kTuftAnchorStep = 9.25f;
  constexpr int kBladesBudgetPerTuftAnchor = 54;
  constexpr float kClumpBladeRadius = 0.44f;
  constexpr float kMiniClumpScatter = 2.35f;
  constexpr unsigned kGrassSeedSalt = 0xC1A55EEDu;

  std::vector<glm::vec4> inst;
  inst.reserve(static_cast<size_t>(kMaxInstances));

  std::mt19937 rng(static_cast<unsigned>(cacheKey_ ^ kGrassSeedSalt ^ 0x9E3779B9u));
  std::uniform_real_distribution<float> rot01(0.0f, kTwoPi);
  std::uniform_real_distribution<float> scaleR(0.88f, 1.14f);
  std::uniform_real_distribution<float> uni01(0.0f, 1.0f);
  std::uniform_int_distribution<int> bladesInClump(3, 18);
  std::uniform_real_distribution<float> scatterXZ(-kMiniClumpScatter, kMiniClumpScatter);

  auto emitBladesAround = [&](float baseX, float baseZ, int count) {
    for (int b = 0; b < count && static_cast<int>(inst.size()) < kMaxInstances; ++b) {
      const float phi = uni01(rng) * kTwoPi;
      const float rr = std::sqrt(uni01(rng)) * kClumpBladeRadius;
      float ox = std::cos(phi) * rr;
      float oz = std::sin(phi) * rr;
      float jx = baseX + ox;
      float jz = baseZ + oz;
      if (!isGrassZone(jx, jz, halfL, halfW, halfGrassL, halfRoadW)) {
        jx = baseX;
        jz = baseZ;
        if (!isGrassZone(jx, jz, halfL, halfW, halfGrassL, halfRoadW)) {
          continue;
        }
      }
      inst.emplace_back(jx, jz, rot01(rng), scaleR(rng));
    }
  };

  const int ni = std::max(6, static_cast<int>(std::ceil((2.0f * halfGrassL) / kTuftAnchorStep)));
  const int nj = std::max(6, static_cast<int>(std::ceil((2.0f * halfGrassW) / kTuftAnchorStep)));
  const float invNi = 1.0f / static_cast<float>(ni);
  const float invNj = 1.0f / static_cast<float>(nj);

  for (int gi = 0; gi < ni && static_cast<int>(inst.size()) < kMaxInstances; ++gi) {
    const float u0 = static_cast<float>(gi) * invNi;
    const float u1 = static_cast<float>(gi + 1) * invNi;
    for (int gj = 0; gj < nj && static_cast<int>(inst.size()) < kMaxInstances; ++gj) {
      const float v0 = static_cast<float>(gj) * invNj;
      const float v1 = static_cast<float>(gj + 1) * invNj;
      /// Losowy punkt w prostokącie komórki — bez regularnych rzędów jak przy sztywnej siatce.
      const float cx = glm::mix(-halfGrassL, halfGrassL, glm::mix(u0, u1, uni01(rng)));
      const float cz = glm::mix(-halfGrassW, halfGrassW, glm::mix(v0, v1, uni01(rng)));

      if (!isGrassZone(cx, cz, halfL, halfW, halfGrassL, halfRoadW)) {
        continue;
      }

      int placedHere = 0;
      while (placedHere < kBladesBudgetPerTuftAnchor && static_cast<int>(inst.size()) < kMaxInstances) {
        const int remaining = kBladesBudgetPerTuftAnchor - placedHere;
        int n = bladesInClump(rng);
        if (remaining >= 2) {
          n = std::clamp(n, 2, std::min(18, remaining));
        } else {
          n = remaining;
        }
        float subX = cx + scatterXZ(rng);
        float subZ = cz + scatterXZ(rng);
        if (!isGrassZone(subX, subZ, halfL, halfW, halfGrassL, halfRoadW)) {
          subX = cx;
          subZ = cz;
        }
        emitBladesAround(subX, subZ, n);
        placedHere += n;
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
                       const glm::vec3& lightDir, float ambient, int shadowMapTextureUnit, float timeSec,
                       float directionalWeight, const glm::vec3& sunColor, const glm::vec3& sunDiskWorldPos,
                       float sunDiskWeight, float sunDiskIntensity, float sunDiskRadius,
                       const glm::vec3& sunDiskColor, int numPointLights, const glm::vec3* pointLights,
                       float pointIntensity, float pointRadius, const glm::vec3& pointColor,
                       float grassPointLightScale) const {
  if (!ready()) {
    return;
  }
  shader.use();
  shader.setMat4("uViewProj", glm::value_ptr(viewProj));
  shader.setMat4("uLightViewProj", glm::value_ptr(lightViewProj));
  shader.setVec3("uLightDir", lightDir.x, lightDir.y, lightDir.z);
  shader.setFloat("uAmbient", ambient);
  shader.setFloat("uDirectionalWeight", directionalWeight);
  shader.setVec3("uSunColor", sunColor.x, sunColor.y, sunColor.z);
  shader.setFloat("uSunDiskWeight", sunDiskWeight);
  shader.setVec3("uSunDiskWorldPos", sunDiskWorldPos.x, sunDiskWorldPos.y, sunDiskWorldPos.z);
  shader.setFloat("uSunDiskIntensity", sunDiskIntensity);
  shader.setFloat("uSunDiskRadius", sunDiskRadius);
  shader.setVec3("uSunDiskColor", sunDiskColor.x, sunDiskColor.y, sunDiskColor.z);
  shader.setInt("uNumPointLights", numPointLights);
  if (pointLights != nullptr && numPointLights > 0) {
    shader.setVec3v("uPointPos", pointLights, numPointLights);
  }
  shader.setVec3("uPointColor", pointColor.x, pointColor.y, pointColor.z);
  shader.setFloat("uPointIntensity", pointIntensity);
  shader.setFloat("uPointRadius", pointRadius);
  shader.setFloat("uGrassPointLightScale", grassPointLightScale);
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
