#include "scene/ScreenRay.hpp"

#include "rendering/Camera.hpp"

#include <glm/gtc/matrix_inverse.hpp>

#include <cmath>

namespace parking {

bool screenToWorldRay(float fx, float fy, int fbW, int fbH, const Camera& cam, glm::vec3& outOrigin,
                      glm::vec3& outDir) {
  if (fbW <= 0 || fbH <= 0) {
    return false;
  }
  const float ndcX = 2.0f * fx / static_cast<float>(fbW) - 1.0f;
  const float ndcY = 1.0f - 2.0f * fy / static_cast<float>(fbH);
  const float aspect = static_cast<float>(fbW) / static_cast<float>(fbH);
  const glm::mat4 vp = cam.projection(aspect) * cam.view();
  const glm::mat4 invVp = glm::inverse(vp);

  glm::vec4 pNear = invVp * glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
  glm::vec4 pFar = invVp * glm::vec4(ndcX, ndcY, 1.0f, 1.0f);
  pNear /= pNear.w;
  pFar /= pFar.w;

  outOrigin = cam.eye();
  outDir = glm::normalize(glm::vec3(pFar - pNear));
  if (!std::isfinite(outDir.x) || glm::length(outDir) < 1e-8f) {
    return false;
  }
  return true;
}

bool rayIntersectPlaneY(const glm::vec3& origin, const glm::vec3& dir, float planeY, glm::vec3& outPoint) {
  const float dy = dir.y;
  if (std::abs(dy) < 1e-7f) {
    return false;
  }
  const float t = (planeY - origin.y) / dy;
  if (t < 0.0f) {
    return false;
  }
  outPoint = origin + t * dir;
  return true;
}

}  // namespace parking
