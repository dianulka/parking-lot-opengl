#pragma once

#include <glm/glm.hpp>

namespace parking {

/// Światło kierunkowe + cień; bez lamp punktowych.
class Lighting {
public:
  [[nodiscard]] glm::vec3 clearColor() const;
  [[nodiscard]] float ambientFactor() const;
  [[nodiscard]] glm::vec3 sunDirection() const;
};

}  // namespace parking
