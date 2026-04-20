#pragma once

namespace parking {

class Application;
class Camera;
class ParkingScene;
class Renderer;

/// Single GLFW window user pointer bundles scene input + resize targets.
struct WindowHooks {
  ParkingScene* scene{nullptr};
  Renderer* renderer{nullptr};
  Camera* camera{nullptr};
  Application* app{nullptr};
};

}  // namespace parking
