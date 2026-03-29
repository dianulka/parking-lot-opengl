#pragma once

#include <glad/gl.h>

#include <glm/glm.hpp>

#include <string>

namespace parking {

class Shader {
public:
  Shader() = default;
  ~Shader();

  Shader(const Shader&) = delete;
  Shader& operator=(const Shader&) = delete;
  Shader(Shader&& other) noexcept;
  Shader& operator=(Shader&& other) noexcept;

  void load(const std::string& vertexPath, const std::string& fragmentPath);

  void use() const;
  void setMat4(const std::string& name, const float* value) const;
  void setMat3(const std::string& name, const float* value) const;
  void setVec3(const std::string& name, float x, float y, float z) const;
  void setVec3v(const std::string& arrayName, const glm::vec3* data, int count) const;
  void setFloat(const std::string& name, float value) const;
  void setInt(const std::string& name, int value) const;
  void setBool(const std::string& name, bool value) const;

  [[nodiscard]] GLuint program() const { return program_; }

private:
  GLuint program_{0};
};

}  // namespace parking
