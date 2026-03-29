#include "gl/GlContext.hpp"

#include <glad/gl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cstdio>

namespace parking {

bool GlContext::loadGlad() {
  const int version = gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress));
  if (version == 0) {
    return false;
  }
  const auto* glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
  std::printf("OpenGL %s (GLAD %d.%d)\n", glVersion ? glVersion : "?",
              GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
  return true;
}

}  // namespace parking
