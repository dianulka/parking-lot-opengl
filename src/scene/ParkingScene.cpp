#include "scene/ParkingScene.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

namespace parking {

ParkingScene::ParkingScene(ParkingGenerator generator) : generator_(std::move(generator)) {}

uint64_t ParkingScene::layoutHash() const {
  const uint32_t lenBits = static_cast<uint32_t>(generator_.length() * 1000.0f);
  const uint32_t spots = static_cast<uint32_t>(generator_.spotCount());
  constexpr uint32_t kPlacementVer = 15u;
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

  auto carOnSpotOk = [&](float x, float z) {
    if (std::abs(x) > halfL + 0.01f) {
      return false;
    }
    if (std::abs(z) < halfA - 0.01f) {
      return false;
    }
    return true;
  };

  struct CarSpot {
    float x{};
    float z{};
    float yaw{};
  };
  std::vector<CarSpot> carSpots;
  carSpots.reserve(static_cast<size_t>(totalSpots));
  for (int i = 0; i < nLeft; ++i) {
    const float x = spotXLeft(i);
    const float z = zLeftCenter;
    if (carOnSpotOk(x, z)) {
      carSpots.push_back({x, z, kYawLeft});
    }
  }
  for (int i = 0; i < nRight; ++i) {
    const float x = spotXRight(i);
    const float z = zRightCenter;
    if (carOnSpotOk(x, z)) {
      carSpots.push_back({x, z, kYawRight});
    }
  }

  const uint64_t seed = layoutHash();
  std::seed_seq seq{static_cast<uint32_t>(seed >> 32u), static_cast<uint32_t>(seed & 0xffffffffu)};
  std::mt19937 rng(seq);
  std::shuffle(carSpots.begin(), carSpots.end(), rng);

  const int placeCount = std::min(targetCars, static_cast<int>(carSpots.size()));
  for (int i = 0; i < placeCount; ++i) {
    const CarSpot& s = carSpots[static_cast<size_t>(i)];
    props_.push_back({PropKind::Car, {s.x, kCarY, s.z}, s.yaw, kCarScale});
  }

  constexpr float kLampScale = 0.88f;
  constexpr float kRoadEndX = 0.94f;
  constexpr float kPi = 3.14159265f;
  constexpr int kLampEveryNSpots = 5;
  const float inset = 1.1f;

  auto yawOuterNegZ = [](float x) { return x < 0.0f ? (0.65f + kPi) : (-0.65f + kPi); };
  auto yawOuterPosZ = [](float x) { return x < 0.0f ? -0.65f : 0.65f; };

  props_.push_back({PropKind::Lamp, {-halfL + inset, 0.0f, -halfW + inset}, 0.65f + kPi, kLampScale});
  props_.push_back({PropKind::Lamp, {halfL - inset, 0.0f, -halfW + inset}, -0.65f + kPi, kLampScale});
  props_.push_back({PropKind::Lamp, {-halfL + inset, 0.0f, halfW - inset}, -0.65f, kLampScale});
  props_.push_back({PropKind::Lamp, {halfL - inset, 0.0f, halfW - inset}, 0.65f, kLampScale});

  props_.push_back({PropKind::Lamp, {-gHalfL * kRoadEndX, 0.0f, 0.0f}, 0.0f, kLampScale});
  props_.push_back({PropKind::Lamp, {gHalfL * kRoadEndX, 0.0f, 0.0f}, 3.14159265f, kLampScale});

  const int nAlong = std::max(nLeft, nRight);
  if (nAlong >= kLampEveryNSpots) {
    const float dxAlong = L / static_cast<float>(nAlong);
    auto xAlong = [&](int i) { return -halfL + (static_cast<float>(i) + 0.5f) * dxAlong; };
    constexpr int kMaxExtraPairs = 21;
    int pairs = 0;
    for (int i = kLampEveryNSpots - 1; i < nAlong && pairs < kMaxExtraPairs; i += kLampEveryNSpots) {
      const float x = xAlong(i);
      props_.push_back({PropKind::Lamp, {x, 0.0f, -halfW + inset}, yawOuterNegZ(x), kLampScale});
      props_.push_back({PropKind::Lamp, {x, 0.0f, halfW - inset}, yawOuterPosZ(x), kLampScale});
      ++pairs;
    }
  }
}

}  // namespace parking
