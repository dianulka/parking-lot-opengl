#include "rendering/Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

namespace parking {

void Camera::setOrbit(float distance, float yawDegrees, float pitchDegrees, glm::vec3 target) {
  distance_ = distance;
  yawDeg_ = yawDegrees;
  pitchDeg_ = pitchDegrees;
  target_ = target;
  clampTarget();
  updateEye();
}

void Camera::setBoundsFromLot(float parkingHalfLength, float parkingHalfWidth, float grassMarginMeters) {
  const float hl = parkingHalfLength + grassMarginMeters;
  const float hw = parkingHalfWidth + grassMarginMeters;
  minX_ = -hl;
  maxX_ = hl;
  minZ_ = -hw;
  maxZ_ = hw;
  clampTarget();
  updateEye();
}

void Camera::setPerspectiveFov(float fovDegrees) { fovDegrees_ = fovDegrees; }

void Camera::panWorldXZ(float dx, float dz) {
  target_.x += dx;
  target_.z += dz;
  clampTarget();
  updateEye();
}

void Camera::orbit(float deltaYawDegrees, float deltaPitchDegrees) {
  yawDeg_ += deltaYawDegrees;
  pitchDeg_ += deltaPitchDegrees;
  // Keep camera above the ground plane: look down at the lot (not from below).
  pitchDeg_ = std::clamp(pitchDeg_, 18.0f, 88.0f);
  updateEye();
}

void Camera::zoom(float deltaDistance) {
  distance_ += deltaDistance;
  distance_ = std::clamp(distance_, 8.0f, 420.0f);
  updateEye();
}

void Camera::clampTarget() {
  target_.x = std::clamp(target_.x, minX_, maxX_);
  target_.z = std::clamp(target_.z, minZ_, maxZ_);
  target_.y = 0.0f;
}

void Camera::updateEye() {
  const float yaw = glm::radians(yawDeg_);
  const float pitch = glm::radians(pitchDeg_);
  const float cp = std::cos(pitch);
  glm::vec3 offset;
  offset.x = distance_ * cp * std::sin(yaw);
  offset.y = distance_ * std::sin(pitch);
  offset.z = distance_ * cp * std::cos(yaw);
  eye_ = target_ + offset;
}

glm::mat4 Camera::view() const { return glm::lookAt(eye_, target_, up_); }

glm::mat4 Camera::projection(float aspect) const {
  return glm::perspective(glm::radians(fovDegrees_), aspect, 0.2f, 600.0f);
}

}  // namespace parking
