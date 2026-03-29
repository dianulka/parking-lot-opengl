#include "app/Application.hpp"

#include "gl/GlContext.hpp"
#include "scene/ParkingGenerator.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <utility>

namespace parking {

namespace {

constexpr float kCameraPanSpeed = 38.0f;
constexpr float kCameraOrbitSpeed = 48.0f;

}  // namespace

Application::Application(ParkingScene&& scene)
    : window_(1280, 720, "Parking prostokątny — OpenGL 3.3"), scene_(std::move(scene)) {
  if (!GlContext::loadGlad()) {
    throw std::runtime_error("gladLoadGL failed");
  }
  glEnable(GL_DEPTH_TEST);

  hooks_.scene = &scene_;
  hooks_.renderer = &renderer_;
  hooks_.camera = &camera_;
  input_.attach(window_.native(), &hooks_);
  glfwSetFramebufferSizeCallback(window_.native(), framebufferSizeCallback);
  glfwSetScrollCallback(window_.native(), scrollCallback);

  const float L = scene_.generator().length();
  const float W = ParkingGenerator::fixedWidth();
  const float mg = ParkingGenerator::grassMarginMeters();
  const float span = std::max(L, W) + 2.0f * mg;
  // Pitch > 0: camera is above the target (sin > 0), natural „map / parking” view.
  camera_.setOrbit(std::max(36.0f, span * 0.38f), 48.0f, 72.0f, glm::vec3(0.0f, 0.0f, 0.0f));
  camera_.setBoundsFromLot(scene_.generator().halfLength(), scene_.generator().halfWidth(), mg);

  int w = 0;
  int h = 0;
  window_.framebufferSize(&w, &h);
  renderer_.resize(w, h);
  renderer_.init();

  std::printf(
      "Sterowanie: WASD — przesuniecie | strzalki — obrot | scroll — zoom | ESC — wyjscie\n"
      "Miejsc: %d  dlugosc: %.1f m\n",
      scene_.generator().spotCount(), static_cast<double>(scene_.generator().length()));
}

void Application::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
  auto* h = static_cast<WindowHooks*>(glfwGetWindowUserPointer(window));
  if (h && h->renderer) {
    h->renderer->resize(width, height);
  }
}

void Application::scrollCallback(GLFWwindow* window, double /*xoffset*/, double yoffset) {
  auto* h = static_cast<WindowHooks*>(glfwGetWindowUserPointer(window));
  if (h && h->camera) {
    h->camera->zoom(static_cast<float>(-yoffset) * 1.1f);
  }
}

void Application::updateCamera(GLFWwindow* window, Camera& camera, float dt) {
  const float pan = kCameraPanSpeed * dt;
  const float orbit = kCameraOrbitSpeed * dt;

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    camera.panWorldXZ(0.0f, -pan);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    camera.panWorldXZ(0.0f, pan);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    camera.panWorldXZ(-pan, 0.0f);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    camera.panWorldXZ(pan, 0.0f);
  }
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
    camera.orbit(-orbit, 0.0f);
  }
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
    camera.orbit(orbit, 0.0f);
  }
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    camera.orbit(0.0f, orbit * 0.45f);
  }
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    camera.orbit(0.0f, -orbit * 0.45f);
  }
}

int Application::run() {
  double last = glfwGetTime();
  while (!window_.shouldClose()) {
    const double now = glfwGetTime();
    const float dt = static_cast<float>(now - last);
    last = now;

    updateCamera(window_.native(), camera_, dt);
    renderer_.draw(scene_, camera_, static_cast<float>(now));

    window_.swapBuffers();
    window_.pollEvents();
  }
  return 0;
}

}  // namespace parking
