#pragma once

#include <glm/glm.hpp>

#include <cstdint>

namespace parking {

enum class LightingMode : uint8_t { Day, Night };

/// Światło kierunkowe + cień; dzień / noc — różne niebo, ambient i kierunek „słońca”.
class Lighting {
public:
  void setMode(LightingMode mode) { mode_ = mode; }
  [[nodiscard]] LightingMode mode() const { return mode_; }

  [[nodiscard]] glm::vec3 clearColor() const;
  [[nodiscard]] float ambientFactor() const;
  [[nodiscard]] glm::vec3 sunDirection() const;

private:
  LightingMode mode_{LightingMode::Night};
};

}  // namespace parking
