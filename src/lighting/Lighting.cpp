#include "lighting/Lighting.hpp"

namespace parking {

glm::vec3 Lighting::clearColor() const {
  if (mode_ == LightingMode::Night) {
    return glm::vec3(0.0f, 0.0f, 0.0f);
  }
  return glm::vec3(0.55f, 0.68f, 0.88f);
}

float Lighting::ambientFactor() const {
  if (mode_ == LightingMode::Night) {
    return 0.0f;
  }
  return 0.42f;
}

float Lighting::directionalLightWeight() const {
  return mode_ == LightingMode::Day ? 1.0f : 0.0f;
}

glm::vec3 Lighting::sunColor() const {
  return glm::vec3(1.0f, 0.993f, 0.978f);
}

glm::vec3 Lighting::sunDirection() const {
  if (mode_ == LightingMode::Night) {
    return glm::normalize(glm::vec3(0.42f, 0.48f, 0.38f));
  }
  return glm::normalize(glm::vec3(0.35f, 0.85f, 0.45f));
}

}  // namespace parking
