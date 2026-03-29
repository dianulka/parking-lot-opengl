#include "rendering/Shader.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace parking {

namespace {

std::string readFile(const std::string& path) {
  std::ifstream f(path);
  if (!f) {
    throw std::runtime_error("Shader: cannot open file: " + path);
  }
  std::ostringstream ss;
  ss << f.rdbuf();
  return ss.str();
}

GLuint compile(GLenum type, const char* src) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &src, nullptr);
  glCompileShader(shader);
  GLint ok = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    GLint len = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    std::vector<char> log(static_cast<size_t>(len));
    glGetShaderInfoLog(shader, len, nullptr, log.data());
    glDeleteShader(shader);
    throw std::runtime_error(std::string("Shader compile error: ") + log.data());
  }
  return shader;
}

}  // namespace

Shader::~Shader() {
  if (program_) {
    glDeleteProgram(program_);
  }
}

Shader::Shader(Shader&& other) noexcept : program_(other.program_) { other.program_ = 0; }

Shader& Shader::operator=(Shader&& other) noexcept {
  if (this != &other) {
    if (program_) {
      glDeleteProgram(program_);
    }
    program_ = other.program_;
    other.program_ = 0;
  }
  return *this;
}

void Shader::load(const std::string& vertexPath, const std::string& fragmentPath) {
  if (program_) {
    glDeleteProgram(program_);
    program_ = 0;
  }

  const std::string vsrc = readFile(vertexPath);
  const std::string fsrc = readFile(fragmentPath);
  GLuint vs = compile(GL_VERTEX_SHADER, vsrc.c_str());
  GLuint fs = compile(GL_FRAGMENT_SHADER, fsrc.c_str());

  program_ = glCreateProgram();
  glAttachShader(program_, vs);
  glAttachShader(program_, fs);
  glLinkProgram(program_);

  glDeleteShader(vs);
  glDeleteShader(fs);

  GLint linked = 0;
  glGetProgramiv(program_, GL_LINK_STATUS, &linked);
  if (!linked) {
    GLint len = 0;
    glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &len);
    std::vector<char> log(static_cast<size_t>(len));
    glGetProgramInfoLog(program_, len, nullptr, log.data());
    glDeleteProgram(program_);
    program_ = 0;
    throw std::runtime_error(std::string("Program link error: ") + log.data());
  }
}

void Shader::use() const { glUseProgram(program_); }

void Shader::setMat4(const std::string& name, const float* value) const {
  const GLint loc = glGetUniformLocation(program_, name.c_str());
  if (loc >= 0) {
    glUniformMatrix4fv(loc, 1, GL_FALSE, value);
  }
}

void Shader::setMat3(const std::string& name, const float* value) const {
  const GLint loc = glGetUniformLocation(program_, name.c_str());
  if (loc >= 0) {
    glUniformMatrix3fv(loc, 1, GL_FALSE, value);
  }
}

void Shader::setVec3(const std::string& name, float x, float y, float z) const {
  const GLint loc = glGetUniformLocation(program_, name.c_str());
  if (loc >= 0) {
    glUniform3f(loc, x, y, z);
  }
}

void Shader::setFloat(const std::string& name, float value) const {
  const GLint loc = glGetUniformLocation(program_, name.c_str());
  if (loc >= 0) {
    glUniform1f(loc, value);
  }
}

void Shader::setVec3v(const std::string& arrayName, const glm::vec3* data, int count) const {
  if (!data || count <= 0) {
    return;
  }
  const std::string indexed = arrayName + "[0]";
  const GLint loc = glGetUniformLocation(program_, indexed.c_str());
  if (loc >= 0) {
    glUniform3fv(loc, count, glm::value_ptr(data[0]));
  }
}

void Shader::setInt(const std::string& name, int value) const {
  const GLint loc = glGetUniformLocation(program_, name.c_str());
  if (loc >= 0) {
    glUniform1i(loc, value);
  }
}

void Shader::setBool(const std::string& name, bool value) const {
  const GLint loc = glGetUniformLocation(program_, name.c_str());
  if (loc >= 0) {
    glUniform1i(loc, value ? 1 : 0);
  }
}

}  // namespace parking
