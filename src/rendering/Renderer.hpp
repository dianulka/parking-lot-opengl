#pragma once

#include <glm/glm.hpp>

#include "lighting/Lighting.hpp"
#include "scene/ParkingGenerator.hpp"

#include "models/GltfModel.hpp"
#include "models/GrassBlades.hpp"

#include <array>
#include "rendering/Camera.hpp"
#include "rendering/Shader.hpp"

#include <glad/gl.h>

namespace parking {

class ParkingScene;

class Renderer {
public:
  Renderer() = default;
  ~Renderer();

  Renderer(const Renderer&) = delete;
  Renderer& operator=(const Renderer&) = delete;

  void init();
  void resize(int width, int height);
  void draw(ParkingScene& scene, Camera& camera, float timeSec, bool parkingSettingsOpen,
            bool carAwaitingDestination);

private:
  void drawOverlayUi(bool parkingSettingsOpen, const ParkingGenerator& gen, LightingMode lightingMode,
                      bool carAwaitingDestination);
  void drawHudCornerOverlay(const glm::mat4& orthoPx, bool parkingSettingsOpen);
  void initShadowMap();
  void renderShadowPass(const ParkingScene& scene, const glm::mat4& lightViewProj, float timeSec);

  Shader flatShader_;
  Shader modelShader_;
  Shader depthShader_;
  Shader groundShader_;
  Shader grassShader_;
  Shader grassDepthShader_;
  Shader skyShader_;
  static constexpr int kCarModelCount = 3;
  std::array<GltfModel, kCarModelCount> carModels_{};
  GltfModel lampModel_{};
  GrassBlades grassBlades_;

  GLuint quadVao_{0};
  GLuint quadVbo_{0};
  GLuint quadEbo_{0};
  GLuint lineVao_{0};
  GLuint lineVbo_{0};
  GLuint skyVao_{0};
  GLuint skyVbo_{0};
  GLuint shadowFbo_{0};
  GLuint shadowTex_{0};
  GLuint whiteTex_{0};
  GLuint grassAlbedoTex_{0};
  GLuint roadAlbedoTex_{0};
  int shadowMapSize_{2048};
  int fbWidth_{0};
  int fbHeight_{0};
};

}  // namespace parking
