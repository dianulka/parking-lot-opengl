#include "scene/ParkingScene.hpp"

#include <algorithm>
#include <cmath>

namespace parking {

ParkingScene::ParkingScene(ParkingGenerator generator) : generator_(std::move(generator)) {}

uint64_t ParkingScene::layoutHash() const {
  const uint32_t lenBits = static_cast<uint32_t>(generator_.length() * 1000.0f);
  const uint32_t spots = static_cast<uint32_t>(generator_.spotCount());
  constexpr uint32_t kPlacementVer = 12u;
  return (static_cast<uint64_t>(lenBits) << 32) | static_cast<uint64_t>(spots ^ (kPlacementVer * 0x9E3779B1u));
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
  props_.clear();

  const auto& gen = generator_;
  const float L = gen.length();
  const float W = ParkingGenerator::fixedWidth();
  const float aisle = ParkingGenerator::aisleWidthMeters();
  const float mg = ParkingGenerator::grassMarginMeters();

  const float halfL = L * 0.5f;
  const float halfW = W * 0.5f;
  const float halfA = aisle * 0.5f;
  const float grassL = L + 2.0f * mg;
  const float gHalfL = grassL * 0.5f;

  const int nLeft = std::max(1, gen.leftRowSpotCount());
  const int nRight = std::max(1, gen.rightRowSpotCount());
  const float dxL = L / static_cast<float>(nLeft);
  const float dxR = L / static_cast<float>(nRight);

  const float zLeftCenter = -(halfW + halfA) * 0.5f;
  const float zRightCenter = (halfW + halfA) * 0.5f;

  auto spotXLeft = [&](int idx) { return -halfL + (static_cast<float>(idx) + 0.5f) * dxL; };
  auto spotXRight = [&](int idx) { return -halfL + (static_cast<float>(idx) + 0.5f) * dxR; };

  constexpr float kCarScale = 1.0f;
  constexpr float kCarY = 0.06f;
  constexpr float kYawLeft = 0.0f;
  constexpr float kYawRight = 3.14159265f;

  const int totalSpots = nLeft + nRight;
  int targetCars = (totalSpots * 2) / 5;
  targetCars = std::max(1, targetCars);
  const int maxCars = std::min(20, std::max(1, totalSpots - 1));
  targetCars = std::min(targetCars, maxCars);

  int carsPlaced = 0;
  int iL = 0;
  int iR = 0;
  auto carOnSpotOk = [&](float x, float z) {
    if (std::abs(x) > halfL + 0.01f) {
      return false;
    }
    if (std::abs(z) < halfA - 0.01f) {
      return false;
    }
    return true;
  };

  while (carsPlaced < targetCars && (iL < nLeft || iR < nRight)) {
    if (iL < nLeft) {
      const float x = spotXLeft(iL);
      const float z = zLeftCenter;
      ++iL;
      if (carOnSpotOk(x, z)) {
        props_.push_back({PropKind::Car, {x, kCarY, z}, kYawLeft, kCarScale});
        ++carsPlaced;
      }
      if (carsPlaced >= targetCars) {
        break;
      }
    }
    if (carsPlaced >= targetCars) {
      break;
    }
    if (iR < nRight) {
      const float x = spotXRight(iR);
      const float z = zRightCenter;
      ++iR;
      if (carOnSpotOk(x, z)) {
        props_.push_back({PropKind::Car, {x, kCarY, z}, kYawRight, kCarScale});
        ++carsPlaced;
      }
    }
  }

  constexpr float kLampScale = 0.88f;
  constexpr float kRoadEndX = 0.94f;
  const float inset = 1.1f;

  props_.push_back({PropKind::Lamp, {-halfL + inset, 0.0f, -halfW + inset}, 0.65f, kLampScale});
  props_.push_back({PropKind::Lamp, {halfL - inset, 0.0f, -halfW + inset}, -0.65f, kLampScale});
  props_.push_back({PropKind::Lamp, {-halfL + inset, 0.0f, halfW - inset}, -0.65f, kLampScale});
  props_.push_back({PropKind::Lamp, {halfL - inset, 0.0f, halfW - inset}, 0.65f, kLampScale});

  props_.push_back({PropKind::Lamp, {-gHalfL * kRoadEndX, 0.0f, 0.0f}, 0.0f, kLampScale});
  props_.push_back({PropKind::Lamp, {gHalfL * kRoadEndX, 0.0f, 0.0f}, 3.14159265f, kLampScale});
}

}  // namespace parking
