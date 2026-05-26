#pragma once

#include "app/WindowHooks.hpp"
#include "core/Window.hpp"
#include "input/Input.hpp"
#include "rendering/Camera.hpp"
#include "rendering/Renderer.hpp"
#include "scene/ParkingScene.hpp"

#include <optional>

struct GLFWwindow;

namespace parking {

class Application {
public:
  explicit Application(ParkingScene&& scene);
  ~Application() = default;

  int run();

  /// Zwraca true, jeśli ESC został zużyty do zamknięcia panelu (nie kończy aplikacji).
  bool consumeEscapeForSettings();

  /// Zwraca true, jeśli ESC anulował wybór auta (nie kończy aplikacji).
  bool consumeEscapeForCarSelection();

private:
  static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
  static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
  static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
  static void updateCamera(GLFWwindow* window, Camera& camera, float dt);

  void applySpotsPerRowFromUi();
  void applyRowCountFromUi();
  void handleParkingSettingsKeys();
  void updateWindowTitle();
  void handleWorldLeftClick(float fx, float fy, int fbW, int fbH);
  void sanitizeCarSelection();

  Window window_;
  ParkingScene scene_;
  Camera camera_;
  Renderer renderer_;
  WindowHooks hooks_{};
  Input input_{};
  bool settingsOpen_{false};
  int uiSpotsPerRow_{16};
  int uiRowCount_{2};
  bool prevBracketLeft_{false};
  bool prevBracketRight_{false};
  bool prevCommaKey_{false};
  bool prevPeriodKey_{false};
  bool prevKeyN_{false};
  std::optional<size_t> selectedCarPropIndex_{};
};

}  // namespace parking
