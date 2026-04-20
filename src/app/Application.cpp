#include "app/Application.hpp"

#include "app/UiConstants.hpp"
#include "gl/GlContext.hpp"
#include "lighting/Lighting.hpp"
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

constexpr char kTitleNormal[] = "Parking prostokątny — OpenGL 3.3";

}  // namespace

Application::Application(ParkingScene&& scene)
    : window_(1280, 720, kTitleNormal), scene_(std::move(scene)) {
  if (!GlContext::loadGlad()) {
    throw std::runtime_error("gladLoadGL failed");
  }
  glEnable(GL_DEPTH_TEST);

  hooks_.scene = &scene_;
  hooks_.renderer = &renderer_;
  hooks_.camera = &camera_;
  hooks_.app = this;
  input_.attach(window_.native(), &hooks_);
  glfwSetFramebufferSizeCallback(window_.native(), framebufferSizeCallback);
  glfwSetMouseButtonCallback(window_.native(), mouseButtonCallback);
  glfwSetScrollCallback(window_.native(), scrollCallback);

  const float L = scene_.generator().length();
  const float W = ParkingGenerator::fixedWidth();
  const float mg = ParkingGenerator::grassMarginMeters();
  const float span = std::max(L, W) + 2.0f * mg;
  // Domyślny widok: z góry z lekką perspektywą, ukośnie na pas jezdni / miejsca (jak pierwszy podgląd).
  const float startDist = std::max(52.0f, span * 0.46f);
  constexpr float kStartYawDeg = 39.0f;
  constexpr float kStartPitchDeg = 61.5f;
  camera_.setOrbit(startDist, kStartYawDeg, kStartPitchDeg, glm::vec3(0.0f, 0.0f, 0.0f));
  camera_.setBoundsFromLot(scene_.generator().halfLength(), scene_.generator().halfWidth(), mg);

  int w = 0;
  int h = 0;
  window_.framebufferSize(&w, &h);
  renderer_.resize(w, h);
  renderer_.init();

  uiSpotCount_ = scene_.generator().spotCount();

  std::printf(
      "Sterowanie: WASD — przesuniecie | strzalki — obrot | scroll — zoom\n"
      "Panel: klik przycisk w lewym górnym rogu — nakładka i okno ustawień. "
      "[ ] — liczba miejsc; N — dzien / noc; ESC — zamknij panel.\n"
      "Miejsc: %d  dlugosc: %.1f m\n",
      scene_.generator().spotCount(), static_cast<double>(scene_.generator().length()));
}

bool Application::consumeEscapeForSettings() {
  if (!settingsOpen_) {
    return false;
  }
  settingsOpen_ = false;
  updateWindowTitle();
  return true;
}

void Application::applySpotCountFromUi() {
  scene_.generator().setSpotCount(uiSpotCount_);
  scene_.generator().syncLengthToSpotCount();
  uiSpotCount_ = scene_.generator().spotCount();
  updateWindowTitle();
}

void Application::updateWindowTitle() {
  if (!settingsOpen_) {
    glfwSetWindowTitle(window_.native(), kTitleNormal);
    return;
  }
  char buf[160];
  const char* lum =
      scene_.lighting().mode() == LightingMode::Day ? "dzien" : "noc";
  std::snprintf(buf, sizeof(buf),
                "USTAWIENIA [%d miejsc | %s]  [ ] N swiatlo  ESC  (%.1f m)",
                uiSpotCount_, lum, static_cast<double>(scene_.generator().length()));
  glfwSetWindowTitle(window_.native(), buf);
}

void Application::handleParkingSettingsKeys() {
  if (!settingsOpen_) {
    prevBracketLeft_ = false;
    prevBracketRight_ = false;
    prevKeyN_ = false;
    return;
  }

  GLFWwindow* w = window_.native();
  const bool left = glfwGetKey(w, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS;
  const bool right = glfwGetKey(w, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS;
  const bool keyN = glfwGetKey(w, GLFW_KEY_N) == GLFW_PRESS;

  if (left && !prevBracketLeft_) {
    uiSpotCount_ =
        std::clamp(uiSpotCount_ - 1, ParkingGenerator::kMinSpots, ParkingGenerator::kMaxSpots);
    applySpotCountFromUi();
  }
  if (right && !prevBracketRight_) {
    uiSpotCount_ =
        std::clamp(uiSpotCount_ + 1, ParkingGenerator::kMinSpots, ParkingGenerator::kMaxSpots);
    applySpotCountFromUi();
  }
  if (keyN && !prevKeyN_) {
    Lighting& lit = scene_.lighting();
    lit.setMode(lit.mode() == LightingMode::Day ? LightingMode::Night : LightingMode::Day);
    updateWindowTitle();
  }

  prevBracketLeft_ = left;
  prevBracketRight_ = right;
  prevKeyN_ = keyN;
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

void Application::mouseButtonCallback(GLFWwindow* window, int button, int action, int /*mods*/) {
  if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS) {
    return;
  }
  auto* h = static_cast<WindowHooks*>(glfwGetWindowUserPointer(window));
  if (!h || !h->app) {
    return;
  }

  double cx = 0.0;
  double cy = 0.0;
  glfwGetCursorPos(window, &cx, &cy);

  int winW = 0;
  int winH = 0;
  glfwGetWindowSize(window, &winW, &winH);
  int fbW = 0;
  int fbH = 0;
  glfwGetFramebufferSize(window, &fbW, &fbH);

  if (winW <= 0 || winH <= 0 || fbW <= 0 || fbH <= 0) {
    return;
  }

  const float sx = static_cast<float>(fbW) / static_cast<float>(winW);
  const float sy = static_cast<float>(fbH) / static_cast<float>(winH);
  const float fx = static_cast<float>(cx) * sx;
  const float fy = static_cast<float>(cy) * sy;

  using parking::ui::kCornerMarginPx;
  using parking::ui::kCornerPanelPx;
  const float xMin = kCornerMarginPx;
  const float xMax = kCornerMarginPx + kCornerPanelPx;
  const float yMinTop = kCornerMarginPx;
  const float yMaxTop = kCornerMarginPx + kCornerPanelPx;

  if (fx >= xMin && fx <= xMax && fy >= yMinTop && fy <= yMaxTop) {
    h->app->settingsOpen_ = !h->app->settingsOpen_;
    h->app->updateWindowTitle();
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

    handleParkingSettingsKeys();
    updateCamera(window_.native(), camera_, dt);

    renderer_.draw(scene_, camera_, static_cast<float>(now), settingsOpen_);

    window_.swapBuffers();
    window_.pollEvents();
  }
  return 0;
}

}  // namespace parking
