#pragma once

#include <glm/glm.hpp>

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
  void draw(ParkingScene& scene, Camera& camera, float timeSec);

private:
  void initShadowMap();
  void renderShadowPass(const ParkingScene& scene, const glm::mat4& lightViewProj, float timeSec);

  Shader flatShader_;
  Shader modelShader_;
  Shader depthShader_;
  Shader groundShader_;
  Shader grassShader_;
  Shader grassDepthShader_;
  std::array<GltfModel, 2> propModels_{};
  GrassBlades grassBlades_;

  GLuint quadVao_{0};
  GLuint quadVbo_{0};
  GLuint quadEbo_{0};
  GLuint lineVao_{0};
  GLuint lineVbo_{0};
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
