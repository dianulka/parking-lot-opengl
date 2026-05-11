#include "rendering/Renderer.hpp"

#include "app/UiConstants.hpp"
#include "lighting/Lighting.hpp"
#include "scene/ParkingGenerator.hpp"
#include "scene/ParkingScene.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdio>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

#define STB_EASY_FONT_IMPLEMENTATION
#include <stb_easy_font.h>

namespace parking {

namespace {

#ifndef PARKING_ASSETS_DIR
#define PARKING_ASSETS_DIR "."
#endif

std::string assetPath(const char* relative) {
  return std::string(PARKING_ASSETS_DIR) + "/" + relative;
}

void appendEasyFontTriangles(float framebufferHeight, float anchorPrintX, float anchorPrintYTopDown, float scale,
                             const char* text, std::vector<float>& out) {
  unsigned char buffer[49152];
  const int numQuads =
      stb_easy_font_print(anchorPrintX, anchorPrintYTopDown, const_cast<char*>(text), nullptr, buffer,
                          static_cast<int>(sizeof(buffer)));
  const float ax = anchorPrintX;
  const float ay = anchorPrintYTopDown;
  for (int q = 0; q < numQuads; ++q) {
    const unsigned char* base = buffer + q * 64;
    float vx[4];
    float vy[4];
    for (int k = 0; k < 4; ++k) {
      const float* p = reinterpret_cast<const float*>(base + k * 16);
      const float xs = ax + (p[0] - ax) * scale;
      const float ys = ay + (p[1] - ay) * scale;
      vx[k] = xs;
      vy[k] = framebufferHeight - ys;
    }
    constexpr int triIdx[6] = {0, 1, 2, 0, 2, 3};
    for (int i = 0; i < 6; ++i) {
      const int k = triIdx[i];
      out.push_back(vx[k]);
      out.push_back(vy[k]);
      out.push_back(0.0f);
    }
  }
}

void flushHudTriangles(Shader& shader, GLuint vao, GLuint vbo, const glm::mat4& ortho,
                       const std::vector<float>& verts, float r, float g, float b, float a = 1.0f) {
  if (verts.empty()) {
    return;
  }
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(verts.size() * sizeof(float)), verts.data(), GL_DYNAMIC_DRAW);
  shader.use();
  shader.setMat4("uMVP", glm::value_ptr(ortho));
  shader.setVec3("uColor", r, g, b);
  shader.setFloat("uAmbient", 1.0f);
  shader.setFloat("uAlpha", a);
  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(verts.size() / 3));
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

void Renderer::drawOverlayUi(bool parkingSettingsOpen, const ParkingGenerator& gen, LightingMode lightingMode,
                             bool carAwaitingDestination) {
  if (fbWidth_ <= 0 || fbHeight_ <= 0) {
    return;
  }

  GLboolean depthWas = GL_TRUE;
  glGetBooleanv(GL_DEPTH_TEST, &depthWas);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  const float fw = static_cast<float>(fbWidth_);
  const float fh = static_cast<float>(fbHeight_);
  const glm::mat4 ortho = glm::ortho(0.0f, fw, 0.0f, fh);

  flatShader_.use();
  glBindVertexArray(lineVao_);
  glBindBuffer(GL_ARRAY_BUFFER, lineVbo_);

  if (parkingSettingsOpen) {
    using parking::ui::kCornerMarginPx;

    static std::vector<float> hudTriTitle;
    static std::vector<float> hudTriBody;
    static std::vector<float> hudTriSmall;

    constexpr float kDimAlpha = 0.38f;

    const float dimVerts[] = {
        0.f,  0.f,  0.f, fw, 0.f, 0.f, fw, fh, 0.f,  //
        0.f,  0.f,  0.f, fw, fh, 0.f, 0.f, fh, 0.f,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(dimVerts), dimVerts, GL_DYNAMIC_DRAW);
    flatShader_.setMat4("uMVP", glm::value_ptr(ortho));
    flatShader_.setVec3("uColor", 0.02f, 0.04f, 0.10f);
    flatShader_.setFloat("uAmbient", 1.0f);
    flatShader_.setFloat("uAlpha", kDimAlpha);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    constexpr float cw = 580.0f;
    constexpr float ch = 348.0f;
    constexpr float titleH = 48.0f;
    constexpr float kFontScale = 2.05f;
    constexpr float kFontScaleSmall = 1.55f;
    constexpr float kPanelFillAlpha = 0.82f;
    constexpr float kPanelBorderAlpha = 0.88f;
    constexpr float kPanelTitleBarAlpha = 0.88f;

    const float mx0 = kCornerMarginPx;
    const float mx1 = mx0 + cw;
    const float my1 = fh - kCornerMarginPx;
    const float my0 = my1 - ch;
    const float ty0 = my1 - titleH;

    const float panelVerts[] = {
        mx0, my0, 0.f, mx1, my0, 0.f, mx1, my1, 0.f,  //
        mx0, my0, 0.f, mx1, my1, 0.f, mx0, my1, 0.f,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(panelVerts), panelVerts, GL_DYNAMIC_DRAW);
    flatShader_.setVec3("uColor", 0.15f, 0.17f, 0.22f);
    flatShader_.setFloat("uAmbient", 1.0f);
    flatShader_.setFloat("uAlpha", kPanelFillAlpha);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    const float borderVerts[] = {
        mx0, my0, 0.f, mx1, my0, 0.f,  //
        mx1, my0, 0.f, mx1, my1, 0.f,  //
        mx1, my1, 0.f, mx0, my1, 0.f,  //
        mx0, my1, 0.f, mx0, my0, 0.f,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(borderVerts), borderVerts, GL_DYNAMIC_DRAW);
    flatShader_.setVec3("uColor", 0.38f, 0.62f, 0.95f);
    flatShader_.setFloat("uAlpha", kPanelBorderAlpha);
    glLineWidth(2.5f);
    glDrawArrays(GL_LINES, 0, 8);
    glLineWidth(1.5f);

    const float titleVerts[] = {
        mx0, ty0, 0.f, mx1, ty0, 0.f, mx1, my1, 0.f,  //
        mx0, ty0, 0.f, mx1, my1, 0.f, mx0, my1, 0.f,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(titleVerts), titleVerts, GL_DYNAMIC_DRAW);
    flatShader_.setVec3("uColor", 0.18f, 0.34f, 0.56f);
    flatShader_.setFloat("uAlpha", kPanelTitleBarAlpha);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    const char* titleStr = "Parking - ustawienia";
    const float titleW =
        static_cast<float>(stb_easy_font_width(const_cast<char*>(titleStr))) * kFontScale;
    const float titleX = mx0 + (cw - titleW) * 0.5f;
    const float titlePrintY = (fh - my1) + (titleH - 12.0f * kFontScale) * 0.5f;
    hudTriTitle.clear();
    appendEasyFontTriangles(fh, titleX, titlePrintY, kFontScale, titleStr, hudTriTitle);
    flushHudTriangles(flatShader_, lineVao_, lineVbo_, ortho, hudTriTitle, 0.96f, 0.98f, 1.0f, 0.96f);

    char line1[160];
    char line2[160];
    char line3[160];
    char line4[160];
    char line5[160];
    std::snprintf(line1, sizeof(line1), "Miejsca postojowe: %d", gen.spotCount());
    std::snprintf(line2, sizeof(line2), "Długość parkingu: %.0f m",
                  static_cast<double>(gen.length()));
    std::snprintf(line3, sizeof(line3), "Oświetlenie: %s  —  klawisz N przełącza dzień / noc",
                  lightingMode == LightingMode::Day ? "dzien" : "noc");
    std::snprintf(line4, sizeof(line4), "Miejsca: [ ] zmiana | ESC — zamknij panel");
    const int smin = ParkingGenerator::kMinSpots;
    const int smax = ParkingGenerator::kMaxSpots;
    std::snprintf(line5, sizeof(line5), "min %d  <--- wiecej w prawo --->  max %d", smin, smax);

    constexpr float padBody = 28.0f;
    const float textLeft = mx0 + padBody;
    float lineY = fh - ty0 + 16.0f;
    hudTriBody.clear();
    appendEasyFontTriangles(fh, textLeft, lineY, kFontScale, line1, hudTriBody);
    lineY += 23.0f * kFontScale;
    appendEasyFontTriangles(fh, textLeft, lineY, kFontScale, line2, hudTriBody);
    lineY += 23.0f * kFontScale;
    appendEasyFontTriangles(fh, textLeft, lineY, kFontScale, line3, hudTriBody);
    lineY += 23.0f * kFontScale;
    appendEasyFontTriangles(fh, textLeft, lineY, kFontScale, line4, hudTriBody);
    flushHudTriangles(flatShader_, lineVao_, lineVbo_, ortho, hudTriBody, 0.76f, 0.82f, 0.92f, 0.94f);

    constexpr float padX = 52.0f;
    constexpr float barH = 30.0f;
    const float bx0 = mx0 + padX;
    const float bx1 = mx1 - padX;
    const float by0 = my0 + 54.0f;
    const float by1 = by0 + barH;

    const float labelPrintY = fh - by1 - 22.0f;
    hudTriSmall.clear();
    appendEasyFontTriangles(fh, textLeft, labelPrintY, kFontScaleSmall, line5, hudTriSmall);
    flushHudTriangles(flatShader_, lineVao_, lineVbo_, ortho, hudTriSmall, 0.52f, 0.60f, 0.74f, 0.88f);

    const float trackVerts[] = {
        bx0, by0, 0.f, bx1, by0, 0.f, bx1, by1, 0.f,  //
        bx0, by0, 0.f, bx1, by1, 0.f, bx0, by1, 0.f,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(trackVerts), trackVerts, GL_DYNAMIC_DRAW);
    flatShader_.setVec3("uColor", 0.20f, 0.22f, 0.27f);
    flatShader_.setFloat("uAmbient", 1.0f);
    flatShader_.setFloat("uAlpha", 0.86f);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    float t = (static_cast<float>(gen.spotCount() - smin)) / static_cast<float>(smax - smin);
    t = std::clamp(t, 0.0f, 1.0f);
    const float span = bx1 - bx0;
    const float fillW = std::max(4.0f, span * t);
    const float fillVerts[] = {
        bx0, by0, 0.f, bx0 + fillW, by0, 0.f, bx0 + fillW, by1, 0.f,  //
        bx0, by0, 0.f, bx0 + fillW, by1, 0.f, bx0, by1, 0.f,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(fillVerts), fillVerts, GL_DYNAMIC_DRAW);
    flatShader_.setVec3("uColor", 0.28f, 0.82f, 0.52f);
    flatShader_.setFloat("uAlpha", 0.92f);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    const float lw = 15.0f;
    const float lh = 24.0f;
    const float lxL = bx0 - 38.0f;
    const float lxR = bx1 + 24.0f;
    const float cyb = (by0 + by1) * 0.5f;
    const float bracketVerts[] = {
        lxL, cyb - lh * 0.5f, 0.f, lxL, cyb + lh * 0.5f, 0.f,  //
        lxL, cyb + lh * 0.5f, 0.f, lxL + lw * 0.4f, cyb + lh * 0.5f, 0.f,
        lxL, cyb - lh * 0.5f, 0.f, lxL + lw * 0.4f, cyb - lh * 0.5f, 0.f,
        lxR + lw, cyb - lh * 0.5f, 0.f, lxR + lw, cyb + lh * 0.5f, 0.f,
        lxR + lw - lw * 0.4f, cyb + lh * 0.5f, 0.f, lxR + lw, cyb + lh * 0.5f, 0.f,
        lxR + lw - lw * 0.4f, cyb - lh * 0.5f, 0.f, lxR + lw, cyb - lh * 0.5f, 0.f,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(bracketVerts), bracketVerts, GL_DYNAMIC_DRAW);
    flatShader_.setVec3("uColor", 0.88f, 0.91f, 0.96f);
    flatShader_.setFloat("uAlpha", 0.9f);
    glLineWidth(2.25f);
    glDrawArrays(GL_LINES, 0, 12);
    glLineWidth(1.5f);
  }

  if (!parkingSettingsOpen && carAwaitingDestination) {
    static std::vector<float> hudTriHint;
    constexpr float kHintScale = 1.35f;
    const char* hint = "Auto wybrane - kliknij wolne miejsce (ESC anuluje)";
    const float textW = static_cast<float>(stb_easy_font_width(const_cast<char*>(hint))) * kHintScale;
    const float textX = std::max(8.0f, (fw - textW) * 0.5f);
    const float textY = fh - 32.0f;
    hudTriHint.clear();
    appendEasyFontTriangles(fh, textX, textY, kHintScale, hint, hudTriHint);
    flushHudTriangles(flatShader_, lineVao_, lineVbo_, ortho, hudTriHint, 0.82f, 0.9f, 1.0f, 0.92f);
  }

  drawHudCornerOverlay(ortho, parkingSettingsOpen);

  glBindVertexArray(0);

  if (depthWas) {
    glEnable(GL_DEPTH_TEST);
  }
  glDisable(GL_BLEND);
}

void Renderer::drawHudCornerOverlay(const glm::mat4& ortho, bool parkingSettingsOpen) {
  using parking::ui::kCornerMarginPx;
  using parking::ui::kCornerPanelPx;

  const float x0 = kCornerMarginPx;
  const float x1 = kCornerMarginPx + kCornerPanelPx;
  const float y0 = static_cast<float>(fbHeight_) - kCornerMarginPx - kCornerPanelPx;
  const float y1 = static_cast<float>(fbHeight_) - kCornerMarginPx;

  const float fillR = parkingSettingsOpen ? 0.22f : 0.14f;
  const float fillG = parkingSettingsOpen ? 0.32f : 0.16f;
  const float fillB = parkingSettingsOpen ? 0.26f : 0.22f;

  const float vertsFill[] = {
      x0, y0, 0.0f, x1, y0, 0.0f, x1, y1, 0.0f,  //
      x0, y0, 0.0f, x1, y1, 0.0f, x0, y1, 0.0f,
  };

  glBindBuffer(GL_ARRAY_BUFFER, lineVbo_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertsFill), vertsFill, GL_DYNAMIC_DRAW);

  flatShader_.use();
  flatShader_.setMat4("uMVP", glm::value_ptr(ortho));
  flatShader_.setVec3("uColor", fillR, fillG, fillB);
  flatShader_.setFloat("uAmbient", 1.0f);
  flatShader_.setFloat("uAlpha", 1.0f);

  glBindVertexArray(lineVao_);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  const float borderR = parkingSettingsOpen ? 0.55f : 0.72f;
  const float borderG = parkingSettingsOpen ? 0.82f : 0.76f;
  const float borderB = parkingSettingsOpen ? 0.62f : 0.95f;

  const float vertsBorder[] = {
      x0, y0, 0.0f, x1, y0, 0.0f,  //
      x1, y0, 0.0f, x1, y1, 0.0f,  //
      x1, y1, 0.0f, x0, y1, 0.0f,  //
      x0, y1, 0.0f, x0, y0, 0.0f,
  };

  glBufferData(GL_ARRAY_BUFFER, sizeof(vertsBorder), vertsBorder, GL_DYNAMIC_DRAW);
  flatShader_.setVec3("uColor", borderR, borderG, borderB);
  flatShader_.setFloat("uAlpha", 1.0f);
  glLineWidth(2.0f);
  glDrawArrays(GL_LINES, 0, 8);
  glLineWidth(1.5f);

  const float cx = (x0 + x1) * 0.5f;
  const float cy = (y0 + y1) * 0.5f;
  const float hw = 14.0f;
  const float vGap = 7.0f;
  const float vertsIcon[] = {
      cx - hw, cy + vGap, 0.0f, cx + hw, cy + vGap, 0.0f,  //
      cx - hw, cy,       0.0f, cx + hw, cy,       0.0f,  //
      cx - hw, cy - vGap, 0.0f, cx + hw, cy - vGap, 0.0f,
  };

  glBufferData(GL_ARRAY_BUFFER, sizeof(vertsIcon), vertsIcon, GL_DYNAMIC_DRAW);
  flatShader_.setVec3("uColor", 0.92f, 0.93f, 0.96f);
  flatShader_.setFloat("uAlpha", 1.0f);
  glDrawArrays(GL_LINES, 0, 6);
}

void Renderer::draw(ParkingScene& scene, Camera& camera, float timeSec, bool parkingSettingsOpen,
                    bool carAwaitingDestination) {
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

  constexpr glm::vec3 kLampBulbLocal(0.1f, 2.42f, 0.36f);
  constexpr int kMaxPointLights = 96;
  std::vector<glm::vec3> lampPointPos;
  lampPointPos.reserve(std::min(scene.props().size(), static_cast<size_t>(kMaxPointLights)));
  for (const PlacedProp& pl : scene.props()) {
    if (pl.kind != PropKind::Lamp) {
      continue;
    }
    if (static_cast<int>(lampPointPos.size()) >= kMaxPointLights) {
      break;
    }
    glm::mat4 lm = glm::translate(glm::mat4(1.0f), pl.position);
    lm = glm::rotate(lm, pl.rotY, glm::vec3(0.0f, 1.0f, 0.0f));
    lm = glm::scale(lm, glm::vec3(pl.scale));
    lampPointPos.push_back(glm::vec3(lm * glm::vec4(kLampBulbLocal, 1.0f)));
  }
  // Lampy punktowe: „moc” vs „zasięg”
  // - Zasięg / jak daleko świeci bez przepaleń kolorów: kPointRadius + w model.frag / ground.frag / grass_inst.frag
  //   wzór att (0.065, 0.72) oraz smoothstep rim (0.48 / 1.28 * uPointRadius).
  // - Jaśniejsze kolory / silniejsze światło przy lampie: kLampPointIntensityScale (+ nocna baza 10.5f wyżej).
  // - Trawa (grass_inst.frag): kGrassNightPointScale — zmniejsza odbicie lamp od źdźbeł w nocy (nie zmienia ziemi).
  // - Dzień: lampy nie świecą (uNumPointLights = 0); modele lamp nadal widoczne.
  constexpr float kLampPointIntensityScale = 0.13f;
  constexpr float kNightLampExtraScale = 1.0f;
  const float pointIntensity =
      (scene.lighting().mode() == LightingMode::Night ? 10.5f : 3.2f) * kLampPointIntensityScale *
      (scene.lighting().mode() == LightingMode::Night ? kNightLampExtraScale : 1.0f);
  const int lampPointLightCount =
      scene.lighting().mode() == LightingMode::Night ? static_cast<int>(lampPointPos.size()) : 0;
  constexpr float kPointRadius = 118.0f;
  constexpr float kGrassNightPointScale = 0.36f;
  const float grassPointLightScale =
      scene.lighting().mode() == LightingMode::Night ? kGrassNightPointScale : 1.0f;

  const glm::vec3 sunColor = scene.lighting().sunColor();
  const float directionalWeight = scene.lighting().directionalLightWeight();
  const glm::vec3 sunDiskWorldPos = lightDir * 680.0f;
  constexpr float kSunDiskIntensity = 1.08f;
  constexpr float kSunDiskRadius = 172.0f;
  const float sunDiskWeight = scene.lighting().mode() == LightingMode::Day ? 1.0f : 0.0f;

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
    groundShader_.setInt("uNumPointLights", lampPointLightCount);
    if (lampPointLightCount > 0) {
      groundShader_.setVec3v("uPointPos", lampPointPos.data(), lampPointLightCount);
    }
    groundShader_.setVec3("uPointColor", 1.0f, 0.93f, 0.72f);
    groundShader_.setFloat("uPointIntensity", pointIntensity);
    groundShader_.setFloat("uPointRadius", kPointRadius);
    groundShader_.setFloat("uDirectionalWeight", directionalWeight);
    groundShader_.setVec3("uSunColor", sunColor.x, sunColor.y, sunColor.z);
    groundShader_.setFloat("uSunDiskWeight", sunDiskWeight);
    groundShader_.setVec3("uSunDiskWorldPos", sunDiskWorldPos.x, sunDiskWorldPos.y, sunDiskWorldPos.z);
    groundShader_.setFloat("uSunDiskIntensity", kSunDiskIntensity);
    groundShader_.setFloat("uSunDiskRadius", kSunDiskRadius);
    groundShader_.setVec3("uSunDiskColor", sunColor.x, sunColor.y, sunColor.z);

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
    grassBlades_.draw(vp, lightVP, grassShader_, lightDir, amb, kShadowUnit, timeSec, directionalWeight, sunColor,
                      sunDiskWorldPos, sunDiskWeight, kSunDiskIntensity, kSunDiskRadius, sunColor,
                      lampPointLightCount,
                      lampPointLightCount > 0 ? lampPointPos.data() : nullptr, pointIntensity, kPointRadius,
                      glm::vec3(1.0f, 0.93f, 0.72f), grassPointLightScale);
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
    flatShader_.setFloat(
        "uAmbient",
        scene.lighting().mode() == LightingMode::Night ? 0.08f : std::max(amb, 0.55f));
    flatShader_.setFloat("uAlpha", 1.0f);

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
    flatShader_.setFloat(
        "uAmbient",
        scene.lighting().mode() == LightingMode::Night ? 0.09f : std::max(amb, 0.58f));
    flatShader_.setFloat("uAlpha", 1.0f);

    glBindVertexArray(lineVao_);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(centerLineVerts.size() / 3));
    glBindVertexArray(0);
    glLineWidth(1.5f);
  }

  glActiveTexture(GL_TEXTURE0 + kShadowUnit);
  glBindTexture(GL_TEXTURE_2D, shadowTex_);

  glActiveTexture(GL_TEXTURE0 + kModelDiffuseUnit);
  glBindTexture(GL_TEXTURE_2D, whiteTex_);

  modelShader_.use();
  modelShader_.setInt("uNumPointLights", lampPointLightCount);
  if (lampPointLightCount > 0) {
    modelShader_.setVec3v("uPointPos", lampPointPos.data(), lampPointLightCount);
  }
  modelShader_.setVec3("uPointColor", 1.0f, 0.93f, 0.72f);
  modelShader_.setFloat("uPointIntensity", pointIntensity);
  modelShader_.setFloat("uPointRadius", kPointRadius);
  modelShader_.setFloat("uDirectionalWeight", directionalWeight);
  modelShader_.setVec3("uSunColor", sunColor.x, sunColor.y, sunColor.z);
  modelShader_.setFloat("uSunDiskWeight", sunDiskWeight);
  modelShader_.setVec3("uSunDiskWorldPos", sunDiskWorldPos.x, sunDiskWorldPos.y, sunDiskWorldPos.z);
  modelShader_.setFloat("uSunDiskIntensity", kSunDiskIntensity);
  modelShader_.setFloat("uSunDiskRadius", kSunDiskRadius);
  modelShader_.setVec3("uSunDiskColor", sunColor.x, sunColor.y, sunColor.z);

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

  drawOverlayUi(parkingSettingsOpen, gen, scene.lighting().mode(), carAwaitingDestination);
}

}  // namespace parking
