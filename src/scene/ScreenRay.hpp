#pragma once

#include <glm/glm.hpp>

namespace parking {

class Camera;

/// Piksel (0,0) lewy górny, jak `fx`/`fy` w framebufferze po skalowaniu z okna GLFW.
bool screenToWorldRay(float fx, float fy, int fbW, int fbH, const Camera& cam, glm::vec3& outOrigin,
                       glm::vec3& outDir);

/// Przecięcie promienia z płaszczyzną równoległą do XZ: y = planeY. Zwraca false, gdy promień jest równoległy
/// lub punkt leży za kamerą (t < 0).
bool rayIntersectPlaneY(const glm::vec3& origin, const glm::vec3& dir, float planeY, glm::vec3& outPoint);

}  // namespace parking
