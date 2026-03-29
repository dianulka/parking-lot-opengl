#pragma once

#include <string>

struct GLFWwindow;

namespace parking {

class Window {
public:
  Window(int width, int height, const std::string& title);
  ~Window();

  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;
  Window(Window&&) = delete;
  Window& operator=(Window&&) = delete;

  [[nodiscard]] GLFWwindow* native() const { return window_; }
  [[nodiscard]] int width() const { return width_; }
  [[nodiscard]] int height() const { return height_; }

  void pollEvents() const;
  [[nodiscard]] bool shouldClose() const;
  void swapBuffers() const;
  void framebufferSize(int* width, int* height) const;

private:
  GLFWwindow* window_{nullptr};
  int width_{0};
  int height_{0};
};

}  // namespace parking
