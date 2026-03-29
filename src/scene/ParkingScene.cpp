#include "scene/ParkingScene.hpp"

#include <cmath>

namespace parking {

ParkingScene::ParkingScene(ParkingGenerator generator) : generator_(std::move(generator)) {}

uint64_t ParkingScene::layoutHash() const {
  const uint32_t lenBits = static_cast<uint32_t>(generator_.length() * 1000.0f);
  const uint32_t spots = static_cast<uint32_t>(generator_.spotCount());
  return (static_cast<uint64_t>(lenBits) << 32) | static_cast<uint64_t>(spots);
}

void ParkingScene::syncPlacements() {
  const uint64_t h = layoutHash();
  if (h == lastLayout_) {
    return;
  }
  lastLayout_ = h;
  rebuildPlacements();
}

void ParkingScene::rebuildPlacements() {
  trees_.clear();

  const auto& gen = generator_;
  const float L = gen.length();
  const float W = ParkingGenerator::fixedWidth();
  const float halfL = L * 0.5f;
  const float halfW = W * 0.5f;

  const float mg = ParkingGenerator::grassMarginMeters();
  const float gHalfL = halfL + mg;
  const float gHalfW = halfW + mg;

  const glm::vec3 corners[] = {
      glm::vec3(-gHalfL * 0.94f, 0.0f, -gHalfW * 0.94f), glm::vec3(gHalfL * 0.94f, 0.0f, -gHalfW * 0.94f),
      glm::vec3(gHalfL * 0.94f, 0.0f, gHalfW * 0.94f),  glm::vec3(-gHalfL * 0.94f, 0.0f, gHalfW * 0.94f),
  };
  for (const glm::vec3& p : corners) {
    trees_.push_back({p, 1.05f});
  }

  trees_.push_back({glm::vec3(0.0f, 0.0f, -gHalfW * 0.97f), 0.95f});
  trees_.push_back({glm::vec3(0.0f, 0.0f, gHalfW * 0.97f), 0.95f});
  trees_.push_back({glm::vec3(-gHalfL * 0.97f, 0.0f, 0.0f), 1.0f});
  trees_.push_back({glm::vec3(gHalfL * 0.97f, 0.0f, 0.0f), 1.0f});
}

}  // namespace parking
