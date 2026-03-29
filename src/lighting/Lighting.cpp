#include "lighting/Lighting.hpp"

namespace parking {

glm::vec3 Lighting::clearColor() const {
  return glm::vec3(0.55f, 0.68f, 0.88f);
}

float Lighting::ambientFactor() const {
  return 0.42f;
}

glm::vec3 Lighting::sunDirection() const {
  return glm::normalize(glm::vec3(0.35f, 0.85f, 0.45f));
}

}  // namespace parking
