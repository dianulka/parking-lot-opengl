#include "core/Window.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stdexcept>

namespace parking {

Window::Window(int width, int height, const std::string& title)
    : width_(width), height_(height) {
  if (!glfwInit()) {
    throw std::runtime_error("glfwInit failed");
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

  window_ = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
  if (!window_) {
    glfwTerminate();
    throw std::runtime_error("glfwCreateWindow failed (OpenGL 3.3 core unavailable?)");
  }

  glfwMakeContextCurrent(window_);
  glfwSwapInterval(1);
}

Window::~Window() {
  if (window_) {
    glfwDestroyWindow(window_);
  }
  glfwTerminate();
}

void Window::pollEvents() const { glfwPollEvents(); }

bool Window::shouldClose() const { return glfwWindowShouldClose(window_) != 0; }

void Window::swapBuffers() const { glfwSwapBuffers(window_); }

void Window::framebufferSize(int* width, int* height) const {
  glfwGetFramebufferSize(window_, width, height);
}

}  // namespace parking
