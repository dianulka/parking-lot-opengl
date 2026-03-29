#pragma once

#include "rendering/Shader.hpp"

#include <glm/glm.hpp>

#include <glad/gl.h>

namespace parking {

/// Procedural tree (trunk + stacked cones) — no external asset.
class TreeMesh {
public:
  TreeMesh() = default;
  ~TreeMesh();

  TreeMesh(const TreeMesh&) = delete;
  TreeMesh& operator=(const TreeMesh&) = delete;

  void create();
  [[nodiscard]] bool ready() const { return vao_ != 0 && trunkIndexCount_ > 0; }

  void draw(const glm::mat4& model, const glm::mat4& viewProj, const glm::mat4& lightViewProj, Shader& shader,
            const glm::vec3& lightDirWorld, const glm::vec3& cameraPos, float ambient, float specularStrength,
            int shadowMapTextureUnit, int diffuseTextureUnit) const;

  void drawShadowCaster(const glm::mat4& model, const glm::mat4& lightViewProj, Shader& depthShader) const;

private:
  GLuint vao_{0};
  GLuint vbo_{0};
  GLuint ebo_{0};
  GLsizei trunkIndexCount_{0};
  GLsizei foliageIndexCount_{0};
};

}  // namespace parking
