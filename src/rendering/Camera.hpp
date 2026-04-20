#pragma once

#include <glm/glm.hpp>

namespace parking {

/// Orbit camera around a ground target (XZ), clamped to a rectangular region.
/// Pitch = elevation above the horizontal plane through the target (degrees): 0 = side view, ~85 = almost top-down.
class Camera {
public:
  void setOrbit(float distance, float yawDegrees, float pitchDegrees, glm::vec3 target);

  void setBoundsFromLot(float parkingHalfLength, float parkingHalfWidth, float grassMarginMeters);
  void setPerspectiveFov(float fovDegrees);

  void panWorldXZ(float dx, float dz);
  void orbit(float deltaYawDegrees, float deltaPitchDegrees);
  void zoom(float deltaDistance);

  void clampTarget();
  void updateEye();

  [[nodiscard]] glm::vec3 eye() const { return eye_; }

  [[nodiscard]] glm::mat4 view() const;
  [[nodiscard]] glm::mat4 projection(float aspect) const;

  [[nodiscard]] float distance() const { return distance_; }

private:
  glm::vec3 eye_{0.0f, 40.0f, 0.1f};
  glm::vec3 target_{0.0f, 0.0f, 0.0f};
  glm::vec3 up_{0.0f, 1.0f, 0.0f};

  float distance_{52.0f};
  float yawDeg_{39.0f};
  float pitchDeg_{61.5f};
  float fovDegrees_{52.0f};

  float minX_{-60.0f};
  float maxX_{60.0f};
  float minZ_{-60.0f};
  float maxZ_{60.0f};
};

}  // namespace parking
