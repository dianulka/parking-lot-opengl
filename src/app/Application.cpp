#include "app/Application.hpp"

#include "app/UiConstants.hpp"
#include "gl/GlContext.hpp"
#include "lighting/Lighting.hpp"
#include "scene/ParkingGenerator.hpp"
#include "scene/ScreenRay.hpp"

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
  const float W = scene_.generator().width();
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

  uiSpotsPerRow_ = scene_.generator().spotsPerRow();
  uiRowCount_ = scene_.generator().rowCount();

  std::printf(
      "Sterowanie: WASD — przesuniecie | strzalki — obrot | scroll — zoom\n"
      "Panel: klik przycisk w lewym górnym rogu — nakładka i okno ustawień. "
      "[ ] — miejsca na rzad; , . — liczba rzedow; N — dzien / noc; G — ukryj dziure; ESC — zamknij panel / anuluj wybor auta.\n"
      "Auto: klik na pojazd, potem klik na wolne miejsce (poza pasem).\n"
      "Miejsc/rzad: %d  rzedy: %d  razem: %d  dlugosc: %.1f m  szerokosc: %.1f m\n",
      scene_.generator().spotsPerRow(), scene_.generator().rowCount(),
      scene_.generator().spotCount(), static_cast<double>(scene_.generator().length()),
      static_cast<double>(scene_.generator().width()));
}

bool Application::consumeEscapeForSettings() {
  if (!settingsOpen_) {
    return false;
  }
  settingsOpen_ = false;
  updateWindowTitle();
  return true;
}

bool Application::consumeEscapeForCarSelection() {
  if (!selectedCarPropIndex_.has_value()) {
    return false;
  }
  selectedCarPropIndex_.reset();
  return true;
}

void Application::sanitizeCarSelection() {
  if (!selectedCarPropIndex_.has_value()) {
    return;
  }
  const size_t i = *selectedCarPropIndex_;
  if (i >= scene_.props().size() || scene_.props()[i].kind != PropKind::Car) {
    selectedCarPropIndex_.reset();
  }
}

void Application::handleWorldLeftClick(float fx, float fy, int fbW, int fbH) {
  sanitizeCarSelection();

  glm::vec3 rayOrigin{};
  glm::vec3 rayDir{};
  if (!screenToWorldRay(fx, fy, fbW, fbH, camera_, rayOrigin, rayDir)) {
    return;
  }

  constexpr float kGroundY = 0.0f;
  glm::vec3 groundPoint{};
  const bool groundHit = rayIntersectPlaneY(rayOrigin, rayDir, kGroundY, groundPoint);

  if (selectedCarPropIndex_.has_value()) {
    if (groundHit &&
        scene_.tryMoveCarToWorldXZ(*selectedCarPropIndex_, groundPoint.x, groundPoint.z)) {
      selectedCarPropIndex_.reset();
      return;
    }
  }

  if (const auto picked = scene_.pickCarPropIndex(rayOrigin, rayDir)) {
    selectedCarPropIndex_ = *picked;
  }
}

void Application::applySpotsPerRowFromUi() {
  selectedCarPropIndex_.reset();
  scene_.generator().setSpotsPerRow(uiSpotsPerRow_);
  scene_.generator().syncLengthToSpotCount();
  uiSpotsPerRow_ = scene_.generator().spotsPerRow();
  updateWindowTitle();
}

void Application::applyRowCountFromUi() {
  selectedCarPropIndex_.reset();
  scene_.generator().setRowCount(uiRowCount_);
  uiRowCount_ = scene_.generator().rowCount();
  const float mg = ParkingGenerator::grassMarginMeters();
  camera_.setBoundsFromLot(scene_.generator().halfLength(), scene_.generator().halfWidth(), mg);
  updateWindowTitle();
}

void Application::updateWindowTitle() {
  if (!settingsOpen_) {
    glfwSetWindowTitle(window_.native(), kTitleNormal);
    return;
  }
  char buf[200];
  const char* lum =
      scene_.lighting().mode() == LightingMode::Day ? "dzien" : "noc";
  std::snprintf(buf, sizeof(buf),
                "USTAWIENIA [%d rzedow x %d miejsc = %d | %s]  [ ] miejsca, , . rzedy, N swiatlo, ESC  (%.1f x %.1f m)",
                uiRowCount_, uiSpotsPerRow_, scene_.generator().spotCount(), lum,
                static_cast<double>(scene_.generator().length()),
                static_cast<double>(scene_.generator().width()));
  glfwSetWindowTitle(window_.native(), buf);
}

void Application::handleParkingSettingsKeys() {
  if (!settingsOpen_) {
    prevBracketLeft_ = false;
    prevBracketRight_ = false;
    prevCommaKey_ = false;
    prevPeriodKey_ = false;
    prevKeyN_ = false;
    return;
  }

  GLFWwindow* w = window_.native();
  const bool left = glfwGetKey(w, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS;
  const bool right = glfwGetKey(w, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS;
  const bool comma = glfwGetKey(w, GLFW_KEY_COMMA) == GLFW_PRESS;
  const bool period = glfwGetKey(w, GLFW_KEY_PERIOD) == GLFW_PRESS;
  const bool keyN = glfwGetKey(w, GLFW_KEY_N) == GLFW_PRESS;

  if (left && !prevBracketLeft_) {
    uiSpotsPerRow_ = std::clamp(uiSpotsPerRow_ - 1, ParkingGenerator::kMinSpotsPerRow,
                                ParkingGenerator::kMaxSpotsPerRow);
    applySpotsPerRowFromUi();
  }
  if (right && !prevBracketRight_) {
    uiSpotsPerRow_ = std::clamp(uiSpotsPerRow_ + 1, ParkingGenerator::kMinSpotsPerRow,
                                ParkingGenerator::kMaxSpotsPerRow);
    applySpotsPerRowFromUi();
  }
  if (comma && !prevCommaKey_) {
    uiRowCount_ = std::clamp(uiRowCount_ - 1, ParkingGenerator::kMinRows,
                             ParkingGenerator::kMaxRows);
    applyRowCountFromUi();
  }
  if (period && !prevPeriodKey_) {
    uiRowCount_ = std::clamp(uiRowCount_ + 1, ParkingGenerator::kMinRows,
                             ParkingGenerator::kMaxRows);
    applyRowCountFromUi();
  }
  if (keyN && !prevKeyN_) {
    Lighting& lit = scene_.lighting();
    lit.setMode(lit.mode() == LightingMode::Day ? LightingMode::Night : LightingMode::Day);
    updateWindowTitle();
  }

  prevBracketLeft_ = left;
  prevBracketRight_ = right;
  prevCommaKey_ = comma;
  prevPeriodKey_ = period;
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
    return;
  }

  h->app->handleWorldLeftClick(fx, fy, fbW, fbH);
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
    if (glfwGetKey(window_.native(), GLFW_KEY_G) == GLFW_PRESS) {
      potholeVisible_ = false;
    }
    updateCamera(window_.native(), camera_, dt);

    sanitizeCarSelection();
    renderer_.draw(scene_, camera_, static_cast<float>(now), settingsOpen_, selectedCarPropIndex_.has_value(),
                   potholeVisible_);

    window_.swapBuffers();
    window_.pollEvents();
  }
  return 0;
}

}  // namespace parking
