#pragma once

struct GLFWwindow;

namespace parking {

struct WindowHooks;

class Input {
public:
  void attach(GLFWwindow* window, WindowHooks* hooks);

  static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
  static WindowHooks* hooks(GLFWwindow* window);
};

}  // namespace parking
