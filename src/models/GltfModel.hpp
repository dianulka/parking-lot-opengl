#pragma once

#include "rendering/Shader.hpp"

#include <glm/glm.hpp>

#include <glad/gl.h>

#include <string>
#include <vector>

namespace parking {

/// Statyczny model z pliku glTF/glB (Assimp) — kilka części (VAO), opcjonalnie tekstury.
class GltfModel {
public:
  GltfModel() = default;
  ~GltfModel();

  GltfModel(const GltfModel&) = delete;
  GltfModel& operator=(const GltfModel&) = delete;

  /// normalizeMaxExtentMeters > 0: skaluje model tak, by największy wymiar == tej wartości (koła na y=0).
  bool loadFromFile(const std::string& path, float normalizeMaxExtentMeters = 0.0f);

  [[nodiscard]] bool ready() const { return !parts_.empty(); }

  void draw(const glm::mat4& model, const glm::mat4& viewProj, const glm::mat4& lightViewProj, Shader& shader,
            const glm::vec3& lightDirWorld, const glm::vec3& cameraPos, float ambient, float specularStrength,
            int shadowMapTextureUnit, int diffuseTextureUnit, GLuint whiteTexture) const;

  void drawShadow(const glm::mat4& model, const glm::mat4& lightViewProj, Shader& depthShader) const;

  /// Wewnętrzna siatka (ładowanie w GltfModel.cpp).
  struct Part {
    GLuint vao{};
    GLuint vbo{};
    GLuint ebo{};
    GLsizei indexCount{};
    GLuint diffuseTex{};
    glm::vec3 baseColor{1.0f};
    bool useTexture{false};
  };

private:
  std::vector<Part> parts_;
};

}  // namespace parking
