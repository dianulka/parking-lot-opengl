#include "input/Input.hpp"

#include "app/Application.hpp"
#include "app/WindowHooks.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace parking {

void Input::attach(GLFWwindow* window, WindowHooks* hooks) {
  glfwSetWindowUserPointer(window, hooks);
  glfwSetKeyCallback(window, keyCallback);
}

WindowHooks* Input::hooks(GLFWwindow* window) {
  return static_cast<WindowHooks*>(glfwGetWindowUserPointer(window));
}

void Input::keyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
  if (action != GLFW_PRESS && action != GLFW_REPEAT) {
    return;
  }
  WindowHooks* h = hooks(window);
  if (!h || !h->scene) {
    return;
  }

  switch (key) {
    case GLFW_KEY_ESCAPE:
      if (h->app && h->app->consumeEscapeForSettings()) {
        break;
      }
      glfwSetWindowShouldClose(window, GLFW_TRUE);
      break;
    default:
      break;
  }
}

}  // namespace parking
