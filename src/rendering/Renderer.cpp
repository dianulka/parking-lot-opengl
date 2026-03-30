#include "rendering/Renderer.hpp"

#include "lighting/Lighting.hpp"
#include "scene/ParkingScene.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdio>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace parking {

namespace {

#ifndef PARKING_ASSETS_DIR
#define PARKING_ASSETS_DIR "."
#endif

std::string assetPath(const char* relative) {
  return std::string(PARKING_ASSETS_DIR) + "/" + relative;
}

void appendLine(std::vector<float>& v, float y, float x0, float z0, float x1, float z1) {
  v.push_back(x0);
  v.push_back(y);
  v.push_back(z0);
  v.push_back(x1);
  v.push_back(y);
  v.push_back(z1);
}

void buildParkingLines(const ParkingGenerator& gen, float y, std::vector<float>& out) {
  out.clear();

  const float L = gen.length();
  const float W = ParkingGenerator::fixedWidth();
  const float aisle = ParkingGenerator::aisleWidthMeters();
  const float halfL = L * 0.5f;
  const float halfW = W * 0.5f;
  const float halfA = aisle * 0.5f;

  const float zLeftOuter = -halfW;
  const float zLeftInner = -halfA;
  const float zRightInner = halfA;
  const float zRightOuter = halfW;

  // Przód i tył płyty (wzdłuż X) — bez linii w środku jezdni
  appendLine(out, y, -halfL, zLeftOuter, halfL, zLeftOuter);
  appendLine(out, y, -halfL, zRightOuter, halfL, zRightOuter);

  // Boki przy x = ±halfL: tylko odcinki przy miejscach (przerwa między = jezdnia)
  appendLine(out, y, -halfL, zLeftOuter, -halfL, zLeftInner);
  appendLine(out, y, -halfL, zRightInner, -halfL, zRightOuter);
  appendLine(out, y, halfL, zLeftOuter, halfL, zLeftInner);
  appendLine(out, y, halfL, zRightInner, halfL, zRightOuter);

  // Granica rząd ↔ pas jezdni
  appendLine(out, y, -halfL, zLeftInner, halfL, zLeftInner);
  appendLine(out, y, -halfL, zRightInner, halfL, zRightInner);

  const int nLeft = std::max(1, gen.leftRowSpotCount());
  const int nRight = std::max(1, gen.rightRowSpotCount());
  const float dxL = L / static_cast<float>(nLeft);
  const float dxR = L / static_cast<float>(nRight);

  for (int i = 0; i <= nLeft; ++i) {
    const float x = -halfL + static_cast<float>(i) * dxL;
    appendLine(out, y, x, zLeftOuter, x, zLeftInner);
  }

  for (int i = 0; i <= nRight; ++i) {
    const float x = -halfL + static_cast<float>(i) * dxR;
    appendLine(out, y, x, zRightInner, x, zRightOuter);
  }
}

GLuint makeTexture2DRgba8(int w, int h, const unsigned char* rgba) {
  GLuint t = 0;
  glGenTextures(1, &t);
  glBindTexture(GL_TEXTURE_2D, t);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glBindTexture(GL_TEXTURE_2D, 0);
  return t;
}

/// Bez mipów: przy płaskim widoku z góry mipmapping rozmazuje proceduralną trawę do jednolitej plamy.
GLuint makeTexture2DRgba8LinearNoMip(int w, int h, const unsigned char* rgba) {
  GLuint t = 0;
  glGenTextures(1, &t);
  glBindTexture(GL_TEXTURE_2D, t);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glBindTexture(GL_TEXTURE_2D, 0);
  return t;
}

void fillRoadAsphaltRgba(int w, int h, std::vector<unsigned char>& out) {
  out.resize(static_cast<size_t>(w * h * 4));
  auto h01 = [](int a, int b) -> float {
    uint32_t n = static_cast<uint32_t>(a) * 374761393u ^ static_cast<uint32_t>(b) * 668265263u;
    n = (n ^ (n >> 13)) * 1274126177u;
    return static_cast<float>(n & 0xFFFFu) / 65535.0f;
  };
  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      const float coarse = h01(x / 4, y / 4);
      const float fine = h01(x, y);
      const float macro = h01(x / 19, y / 19);
      float mix = coarse * 0.45f + fine * 0.35f + macro * 0.2f;
      const float streak = 0.5f + 0.5f * std::sin(static_cast<float>(x + y) * 0.11f);
      mix = mix * 0.82f + streak * 0.18f;
      const float r = 46.0f + mix * 58.0f;
      const float g = 0.96f * r;
      const float b = std::min(255.0f, r + 14.0f);
      const size_t i = static_cast<size_t>(y * w + x) * 4;
      out[i + 0] = static_cast<unsigned char>(std::clamp(r, 0.0f, 255.0f));
      out[i + 1] = static_cast<unsigned char>(std::clamp(g, 0.0f, 255.0f));
      out[i + 2] = static_cast<unsigned char>(std::clamp(b, 0.0f, 255.0f));
      out[i + 3] = 255;
    }
  }
}

/// Krótka trawa: FBM + szczegół „kępek”, plamy jak cień / AO, lekkie rozświetlenia.
void fillGrassRgba(int w, int h, std::vector<unsigned char>& out) {
  out.resize(static_cast<size_t>(w * h * 4));
  auto h01 = [](int a, int b) -> float {
    uint32_t n = static_cast<uint32_t>(a) * 374761393u ^ static_cast<uint32_t>(b) * 668265263u;
    n = (n ^ (n >> 13)) * 1274126177u;
    return static_cast<float>(n & 0xFFFFu) / 65535.0f;
  };

  auto smoothNoise = [&h01](float x, float y) -> float {
    const int x0 = static_cast<int>(std::floor(x));
    const int y0 = static_cast<int>(std::floor(y));
    const float fx = x - static_cast<float>(x0);
    const float fy = y - static_cast<float>(y0);
    const float sx = fx * fx * (3.0f - 2.0f * fx);
    const float sy = fy * fy * (3.0f - 2.0f * fy);
    const float n00 = h01(x0, y0);
    const float n10 = h01(x0 + 1, y0);
    const float n01 = h01(x0, y0 + 1);
    const float n11 = h01(x0 + 1, y0 + 1);
    const float nx0 = n00 + sx * (n10 - n00);
    const float nx1 = n01 + sx * (n11 - n01);
    return nx0 + sy * (nx1 - nx0);
  };

  auto fbm = [&smoothNoise](float x, float y) -> float {
    float v = 0.0f;
    float a = 0.5f;
    float xp = x;
    float yp = y;
    for (int i = 0; i < 5; ++i) {
      v += a * smoothNoise(xp, yp);
      xp *= 2.0f;
      yp *= 2.0f;
      a *= 0.5f;
    }
    return v;
  };

  for (int py = 0; py < h; ++py) {
    for (int px = 0; px < w; ++px) {
      const float fx = static_cast<float>(px) * 0.11f;
      const float fy = static_cast<float>(py) * 0.11f;

      const float large = fbm(fx * 0.35f, fy * 0.35f);
      const float tufts = fbm(fx * 2.5f + 13.7f, fy * 2.5f - 9.1f);
      const float shortGrass = fbm(fx * 7.0f, fy * 7.0f);
      const float micro = h01(px + px / 3, py + py / 5);

      // „Cień” między kępkami: ciemniej gdy large niski
      const float ao = 0.42f + 0.58f * large;
      const float shadow = 0.55f + 0.45f * std::clamp(tufts * 0.7f + shortGrass * 0.3f, 0.0f, 1.0f);
      const float shade = std::clamp(ao * shadow, 0.0f, 1.0f);

      // Lekkie „ostrza” krótkiej trawy (anizotropowy szum)
      const float blade =
          0.5f + 0.5f * std::sin(fx * 14.2f + fy * 8.3f) * (0.15f + 0.85f * shortGrass);

      float g = 28.0f + 120.0f * shade;
      g += 28.0f * std::max(0.0f, tufts - 0.45f) * blade;
      g += 10.0f * (micro - 0.5f);

      float r = g * 0.34f + 22.0f * blade * shade;
      float b = g * 0.30f;

      const float cool = 1.0f - shade;
      b += 28.0f * cool * 0.4f;
      r -= 12.0f * cool * 0.4f;

      const size_t i = static_cast<size_t>(py * w + px) * 4;
      out[i + 0] = static_cast<unsigned char>(std::clamp(r, 0.0f, 255.0f));
      out[i + 1] = static_cast<unsigned char>(std::clamp(g, 0.0f, 255.0f));
      out[i + 2] = static_cast<unsigned char>(std::clamp(b, 0.0f, 255.0f));
      out[i + 3] = 255;
    }
  }
}

}  // namespace

Renderer::~Renderer() {
  if (quadVao_) {
    glDeleteVertexArrays(1, &quadVao_);
  }
  if (quadVbo_) {
    glDeleteBuffers(1, &quadVbo_);
  }
  if (quadEbo_) {
    glDeleteBuffers(1, &quadEbo_);
  }
  if (lineVao_) {
    glDeleteVertexArrays(1, &lineVao_);
  }
  if (lineVbo_) {
    glDeleteBuffers(1, &lineVbo_);
  }
  if (shadowFbo_) {
    glDeleteFramebuffers(1, &shadowFbo_);
  }
  if (shadowTex_) {
    glDeleteTextures(1, &shadowTex_);
  }
  if (whiteTex_) {
    glDeleteTextures(1, &whiteTex_);
  }
  if (grassAlbedoTex_ && grassAlbedoTex_ != whiteTex_) {
    glDeleteTextures(1, &grassAlbedoTex_);
  }
  if (roadAlbedoTex_ && roadAlbedoTex_ != whiteTex_) {
    glDeleteTextures(1, &roadAlbedoTex_);
  }
}

void Renderer::initShadowMap() {
  glGenFramebuffers(1, &shadowFbo_);
  glGenTextures(1, &shadowTex_);
  glBindTexture(GL_TEXTURE_2D, shadowTex_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapSize_, shadowMapSize_, 0, GL_DEPTH_COMPONENT,
               GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  const float border[] = {1.0f, 1.0f, 1.0f, 1.0f};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);

  glBindFramebuffer(GL_FRAMEBUFFER, shadowFbo_);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex_, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::renderShadowPass(const ParkingScene& scene, const glm::mat4& lightViewProj, float timeSec) {
  glViewport(0, 0, shadowMapSize_, shadowMapSize_);
  glBindFramebuffer(GL_FRAMEBUFFER, shadowFbo_);
  glClear(GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  if (grassBlades_.ready()) {
    grassBlades_.drawShadow(lightViewProj, grassDepthShader_, timeSec);
  }

  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(1.25f, 4.0f);

  for (const PlacedProp& p : scene.props()) {
    const int k = static_cast<int>(p.kind);
    if (k < 0 || k >= static_cast<int>(propModels_.size()) || !propModels_[static_cast<size_t>(k)].ready()) {
      continue;
    }
    glm::mat4 m = glm::translate(glm::mat4(1.0f), p.position);
    m = glm::rotate(m, p.rotY, glm::vec3(0.0f, 1.0f, 0.0f));
    m = glm::scale(m, glm::vec3(p.scale));
    propModels_[static_cast<size_t>(k)].drawShadow(m, lightViewProj, depthShader_);
  }

  glDisable(GL_POLYGON_OFFSET_FILL);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::init() {
  flatShader_.load(assetPath("shaders/basic.vert"), assetPath("shaders/basic.frag"));
  modelShader_.load(assetPath("shaders/model.vert"), assetPath("shaders/model.frag"));
  depthShader_.load(assetPath("shaders/depth.vert"), assetPath("shaders/depth.frag"));
  grassShader_.load(assetPath("shaders/grass_inst.vert"), assetPath("shaders/grass_inst.frag"));
  grassDepthShader_.load(assetPath("shaders/grass_depth.vert"), assetPath("shaders/depth.frag"));
  groundShader_.load(assetPath("shaders/ground.vert"), assetPath("shaders/ground.frag"));
  {
    const GLuint p = groundShader_.program();
    const char* names[] = {"uShadowMap", "uGrassAlbedo", "uRoadAlbedo"};
    for (const char* name : names) {
      if (glGetUniformLocation(p, name) < 0) {
        std::fprintf(stderr, "ground shader: brak uniformu \"%s\" (samplery / tekstury)\n", name);
      }
    }
  }

  initShadowMap();

  {
    const unsigned char px[] = {255, 255, 255, 255};
    whiteTex_ = makeTexture2DRgba8(1, 1, px);
    constexpr int kGrassTexSize = 1024;
    std::vector<unsigned char> grassRgba;
    fillGrassRgba(kGrassTexSize, kGrassTexSize, grassRgba);
    grassAlbedoTex_ = makeTexture2DRgba8LinearNoMip(kGrassTexSize, kGrassTexSize, grassRgba.data());
    std::vector<unsigned char> roadRgba;
    fillRoadAsphaltRgba(512, 512, roadRgba);
    roadAlbedoTex_ = makeTexture2DRgba8(512, 512, roadRgba.data());
  }

  constexpr float vertices[] = {
      -0.5f, 0.0f, -0.5f,  //
      0.5f,  0.0f, -0.5f,  //
      0.5f,  0.0f, 0.5f,   //
      -0.5f, 0.0f, 0.5f,   //
  };
  constexpr unsigned int indices[] = {0, 1, 2, 2, 3, 0};

  glGenVertexArrays(1, &quadVao_);
  glGenBuffers(1, &quadVbo_);
  glGenBuffers(1, &quadEbo_);

  glBindVertexArray(quadVao_);

  glBindBuffer(GL_ARRAY_BUFFER, quadVbo_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEbo_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);

  glGenVertexArrays(1, &lineVao_);
  glGenBuffers(1, &lineVbo_);
  glBindVertexArray(lineVao_);
  glBindBuffer(GL_ARRAY_BUFFER, lineVbo_);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);

  static const char* kPropRel[] = {
      "models/1985_toyota_sprinter_trueno_ae86.glb",  // Car
      "models/scifi_lamp.glb",                        // Lamp
  };
  propModels_[0].loadFromFile(assetPath(kPropRel[0]), 4.5f);
  propModels_[1].loadFromFile(assetPath(kPropRel[1]), 2.8f);

  grassBlades_.create();
}

void Renderer::resize(int width, int height) {
  fbWidth_ = width;
  fbHeight_ = height;
  glViewport(0, 0, width, height);
}

void Renderer::draw(ParkingScene& scene, Camera& camera, float timeSec) {
  if (fbWidth_ <= 0 || fbHeight_ <= 0) {
    return;
  }

  scene.syncPlacements();

  const auto& gen = scene.generator();
  camera.setBoundsFromLot(gen.halfLength(), gen.halfWidth(), ParkingGenerator::grassMarginMeters());
  camera.updateEye();

  grassBlades_.rebuildIfNeeded(gen);

  const float L = gen.length();
  const float W = ParkingGenerator::fixedWidth();
  const float aisle = ParkingGenerator::aisleWidthMeters();
  const float mg = ParkingGenerator::grassMarginMeters();
  const float grassL = L + 2.0f * mg;
  const float grassW = W + 2.0f * mg;
  const float halfGrassL = grassL * 0.5f;

  constexpr float kGrassRepeatMeters = 10.0f;
  const float grassTex = 1.0f / kGrassRepeatMeters;
  const int perRow = std::max(1, gen.spotsPerRow());
  const float spotW = L / static_cast<float>(perRow);
  const float roadTex = 3.4f / std::max(spotW, 3.5f);
  const float roadW = std::max(aisle * 1.12f, 9.5f);

  const float aspect = static_cast<float>(fbWidth_) / static_cast<float>(fbHeight_);
  const glm::mat4 vp = camera.projection(aspect) * camera.view();

  const float amb = scene.lighting().ambientFactor();
  const glm::vec3 lightDir = glm::normalize(scene.lighting().sunDirection());
  const glm::vec3 camPos = camera.eye();

  const float orthoExtent = std::max(grassL, grassW) * 0.5f + 22.0f;
  glm::vec3 center(0.0f);
  glm::vec3 eye = center + lightDir * (orthoExtent * 2.8f);
  glm::vec3 up = std::abs(lightDir.y) > 0.92f ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
  const glm::mat4 lightView = glm::lookAt(eye, center, up);
  const glm::mat4 lightProj = glm::ortho(-orthoExtent, orthoExtent, -orthoExtent, orthoExtent, 2.0f, orthoExtent * 4.0f);
  const glm::mat4 lightVP = lightProj * lightView;

  renderShadowPass(scene, lightVP, timeSec);

  glViewport(0, 0, fbWidth_, fbHeight_);
  const glm::vec3 bg = scene.lighting().clearColor();
  glClearColor(bg.r, bg.g, bg.b, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_DEPTH_TEST);
  glLineWidth(1.5f);

  constexpr int kShadowUnit = 0;
  constexpr int kModelDiffuseUnit = 1;
  constexpr int kGrassTexUnit = 2;
  constexpr int kRoadTexUnit = 3;

  glActiveTexture(GL_TEXTURE0 + kShadowUnit);
  glBindTexture(GL_TEXTURE_2D, shadowTex_);

  // Jedna płaszczyzna terenu — w shaderze: parking / pas drogi / trawa (bez nakładających się quadów).
  {
    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(grassL, 1.0f, grassW));
    groundShader_.use();
    groundShader_.setMat4("uModel", glm::value_ptr(model));
    groundShader_.setMat4("uViewProj", glm::value_ptr(vp));
    groundShader_.setMat4("uLightViewProj", glm::value_ptr(lightVP));
    groundShader_.setVec3("uGrassTint", 0.55f, 0.95f, 0.48f);
    groundShader_.setVec3("uRoadStripTint", 0.58f, 0.58f, 0.6f);
    groundShader_.setVec3("uParkingTint", 0.62f, 0.62f, 0.64f);
    groundShader_.setFloat("uAmbient", amb);
    groundShader_.setVec3("uLightDir", lightDir.x, lightDir.y, lightDir.z);
    groundShader_.setInt("uShadowMap", kShadowUnit);
    groundShader_.setInt("uGrassAlbedo", kGrassTexUnit);
    groundShader_.setInt("uRoadAlbedo", kRoadTexUnit);
    groundShader_.setFloat("uGrassTexScale", grassTex);
    groundShader_.setFloat("uRoadTexScale", roadTex);
    groundShader_.setFloat("uRoadTexScaleStrip", roadTex * 0.85f);
    groundShader_.setFloat("uHalfParkingL", L * 0.5f);
    groundShader_.setFloat("uHalfParkingW", W * 0.5f);
    groundShader_.setFloat("uHalfRoadW", roadW * 0.5f);
    groundShader_.setFloat("uHalfGrassL", grassL * 0.5f);

    glActiveTexture(GL_TEXTURE0 + kGrassTexUnit);
    glBindTexture(GL_TEXTURE_2D, grassAlbedoTex_);
    glActiveTexture(GL_TEXTURE0 + kRoadTexUnit);
    glBindTexture(GL_TEXTURE_2D, roadAlbedoTex_);
    glActiveTexture(GL_TEXTURE0 + kShadowUnit);
    glBindTexture(GL_TEXTURE_2D, shadowTex_);

    glBindVertexArray(quadVao_);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
  }

  if (grassBlades_.ready()) {
    glActiveTexture(GL_TEXTURE0 + kShadowUnit);
    glBindTexture(GL_TEXTURE_2D, shadowTex_);
    grassBlades_.draw(vp, lightVP, grassShader_, lightDir, amb, kShadowUnit, timeSec);
  }

  static std::vector<float> lineVerts;
  static std::vector<float> centerLineVerts;
  buildParkingLines(gen, 0.09f, lineVerts);
  centerLineVerts.clear();
  {
    const float yAx = 0.098f;
    appendLine(centerLineVerts, yAx, -halfGrassL, 0.0f, halfGrassL, 0.0f);
  }

  if (!lineVerts.empty()) {
    glBindBuffer(GL_ARRAY_BUFFER, lineVbo_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(lineVerts.size() * sizeof(float)), lineVerts.data(),
                 GL_DYNAMIC_DRAW);

    flatShader_.use();
    flatShader_.setMat4("uMVP", glm::value_ptr(vp));
    flatShader_.setVec3("uColor", 0.92f, 0.92f, 0.85f);
    flatShader_.setFloat("uAmbient", std::max(amb, 0.55f));

    glBindVertexArray(lineVao_);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(lineVerts.size() / 3));
    glBindVertexArray(0);
  }

  if (!centerLineVerts.empty()) {
    glLineWidth(2.25f);
    glBindBuffer(GL_ARRAY_BUFFER, lineVbo_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(centerLineVerts.size() * sizeof(float)),
                 centerLineVerts.data(), GL_DYNAMIC_DRAW);

    flatShader_.use();
    flatShader_.setMat4("uMVP", glm::value_ptr(vp));
    flatShader_.setVec3("uColor", 0.92f, 0.82f, 0.18f);
    flatShader_.setFloat("uAmbient", std::max(amb, 0.58f));

    glBindVertexArray(lineVao_);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(centerLineVerts.size() / 3));
    glBindVertexArray(0);
    glLineWidth(1.5f);
  }

  glActiveTexture(GL_TEXTURE0 + kShadowUnit);
  glBindTexture(GL_TEXTURE_2D, shadowTex_);

  glActiveTexture(GL_TEXTURE0 + kModelDiffuseUnit);
  glBindTexture(GL_TEXTURE_2D, whiteTex_);
  for (const PlacedProp& p : scene.props()) {
    const int k = static_cast<int>(p.kind);
    if (k < 0 || k >= static_cast<int>(propModels_.size()) || !propModels_[static_cast<size_t>(k)].ready()) {
      continue;
    }
    float spec = 0.1f;
    if (p.kind == PropKind::Car) {
      spec = 0.16f;
    } else if (p.kind == PropKind::Lamp) {
      spec = 0.45f;
    }
    glm::mat4 m = glm::translate(glm::mat4(1.0f), p.position);
    m = glm::rotate(m, p.rotY, glm::vec3(0.0f, 1.0f, 0.0f));
    m = glm::scale(m, glm::vec3(p.scale));
    propModels_[static_cast<size_t>(k)].draw(m, vp, lightVP, modelShader_, lightDir, camPos, amb, spec, kShadowUnit,
                                             kModelDiffuseUnit, whiteTex_);
  }

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

}  // namespace parking
