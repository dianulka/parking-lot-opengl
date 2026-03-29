#pragma once

#include "app/WindowHooks.hpp"
#include "core/Window.hpp"
#include "input/Input.hpp"
#include "rendering/Camera.hpp"
#include "rendering/Renderer.hpp"
#include "scene/ParkingScene.hpp"

struct GLFWwindow;

namespace parking {

class Application {
public:
  explicit Application(ParkingScene&& scene);
  int run();

private:
  static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
  static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
  static void updateCamera(GLFWwindow* window, Camera& camera, float dt);

  Window window_;
  ParkingScene scene_;
  Camera camera_;
  Renderer renderer_;
  WindowHooks hooks_{};
  Input input_{};
};

}  // namespace parking
