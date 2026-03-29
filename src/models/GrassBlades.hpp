#pragma once

#include "rendering/Shader.hpp"

#include <glm/glm.hpp>

#include <glad/gl.h>

namespace parking {

class ParkingGenerator;

/// Krótkie źdźbła (dwa krzyżujące się quady) — tylko strefa trawnika (poza L×W i pasem drogi).
class GrassBlades {
public:
  GrassBlades() = default;
  ~GrassBlades();

  GrassBlades(const GrassBlades&) = delete;
  GrassBlades& operator=(const GrassBlades&) = delete;

  void create();
  void rebuildIfNeeded(const ParkingGenerator& gen);

  [[nodiscard]] bool ready() const { return vao_ != 0 && indexCount_ > 0 && instanceCount_ > 0; }

  void draw(const glm::mat4& viewProj, const glm::mat4& lightViewProj, Shader& shader, const glm::vec3& lightDir,
            float ambient, int shadowMapTextureUnit, float timeSec) const;

  void drawShadow(const glm::mat4& lightViewProj, Shader& depthShader, float timeSec) const;

private:
  GLuint vao_{0};
  GLuint vbo_{0};
  GLuint ebo_{0};
  GLuint instanceVbo_{0};
  GLsizei indexCount_{0};
  GLsizei instanceCount_{0};
  unsigned cacheKey_{0};
};

}  // namespace parking
