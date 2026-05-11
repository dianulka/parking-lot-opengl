#include "scene/ParkingScene.hpp"

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <vector>

namespace parking {

namespace {

constexpr float kCarY = 0.06f;
constexpr float kYawLeft = 0.0f;
constexpr float kYawRight = 3.14159265f;

/// Pudełko pick w przestrzeni modelu (auto znormalizowane ~4.5 m); środek modelu ~środek pojazdu.
const glm::vec3 kCarPickHalfExtents(2.15f, 0.52f, 1.0f);

struct LotGeom {
  float L{};
  float halfL{};
  float halfW{};
  float halfA{};
  int nLeft{};
  int nRight{};
  float dxL{};
  float dxR{};
  float zLeftCenter{};
  float zRightCenter{};
  float grassL{};
  float gHalfL{};
};

void fillLotGeom(const ParkingGenerator& gen, LotGeom& g) {
  g.L = gen.length();
  const float W = ParkingGenerator::fixedWidth();
  const float aisle = ParkingGenerator::aisleWidthMeters();
  const float mg = ParkingGenerator::grassMarginMeters();
  g.halfL = g.L * 0.5f;
  g.halfW = W * 0.5f;
  g.halfA = aisle * 0.5f;
  g.grassL = g.L + 2.0f * mg;
  g.gHalfL = g.grassL * 0.5f;
  g.nLeft = std::max(1, gen.leftRowSpotCount());
  g.nRight = std::max(1, gen.rightRowSpotCount());
  g.dxL = g.L / static_cast<float>(g.nLeft);
  g.dxR = g.L / static_cast<float>(g.nRight);
  g.zLeftCenter = -(g.halfW + g.halfA) * 0.5f;
  g.zRightCenter = (g.halfW + g.halfA) * 0.5f;
}

bool carOnSpotOk(const LotGeom& g, float x, float z) {
  if (std::abs(x) > g.halfL + 0.01f) {
    return false;
  }
  if (std::abs(z) < g.halfA - 0.01f) {
    return false;
  }
  return true;
}

bool raySlabAabb(const glm::vec3& o, const glm::vec3& d, const glm::vec3& bmin, const glm::vec3& bmax, float& tHit) {
  float tMin = 0.0f;
  float tMax = std::numeric_limits<float>::max();
  constexpr float kEps = 1e-8f;
  for (int i = 0; i < 3; ++i) {
    if (std::abs(d[i]) < kEps) {
      if (o[i] < bmin[i] || o[i] > bmax[i]) {
        return false;
      }
    } else {
      const float invD = 1.0f / d[i];
      float t1 = (bmin[i] - o[i]) * invD;
      float t2 = (bmax[i] - o[i]) * invD;
      if (t1 > t2) {
        std::swap(t1, t2);
      }
      tMin = std::max(tMin, t1);
      tMax = std::min(tMax, t2);
      if (tMin > tMax) {
        return false;
      }
    }
  }
  if (tMax < 0.0f) {
    return false;
  }
  tHit = (tMin >= 0.0f) ? tMin : tMax;
  return std::isfinite(tHit) && tHit >= 0.0f;
}

bool snapHitToSpot(const LotGeom& g, float hitX, float hitZ, float& outX, float& outZ, float& outYaw) {
  constexpr float kAisleMargin = 0.02f;
  if (std::abs(hitZ) < g.halfA - kAisleMargin) {
    return false;
  }
  const float distL = std::abs(hitZ - g.zLeftCenter);
  const float distR = std::abs(hitZ - g.zRightCenter);
  const bool useLeft = distL <= distR;
  const int n = useLeft ? g.nLeft : g.nRight;
  const float dx = useLeft ? g.dxL : g.dxR;
  const float t = (hitX + g.halfL) / dx - 0.5f;
  int idx = static_cast<int>(std::lround(t));
  idx = std::clamp(idx, 0, n - 1);
  outX = -g.halfL + (static_cast<float>(idx) + 0.5f) * dx;
  outZ = useLeft ? g.zLeftCenter : g.zRightCenter;
  outYaw = useLeft ? kYawLeft : kYawRight;
  return carOnSpotOk(g, outX, outZ);
}

}  // namespace

ParkingScene::ParkingScene(ParkingGenerator generator) : generator_(std::move(generator)) {}

uint64_t ParkingScene::layoutHash() const {
  const uint32_t lenBits = static_cast<uint32_t>(generator_.length() * 1000.0f);
  const uint32_t spots = static_cast<uint32_t>(generator_.spotCount());
  constexpr uint32_t kPlacementVer = 19u;
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
  LotGeom lg{};
  fillLotGeom(gen, lg);

  const float halfL = lg.halfL;
  const float halfW = lg.halfW;
  const float halfA = lg.halfA;
  const float gHalfL = lg.gHalfL;
  const int nLeft = lg.nLeft;
  const int nRight = lg.nRight;
  const float dxL = lg.dxL;
  const float dxR = lg.dxR;
  const float zLeftCenter = lg.zLeftCenter;
  const float zRightCenter = lg.zRightCenter;

  auto spotXLeft = [&](int idx) { return -halfL + (static_cast<float>(idx) + 0.5f) * dxL; };
  auto spotXRight = [&](int idx) { return -halfL + (static_cast<float>(idx) + 0.5f) * dxR; };

  constexpr float kCarScale = 1.0f;

  const int totalSpots = nLeft + nRight;
  int targetCars = (totalSpots * 2) / 5;
  targetCars = std::max(1, targetCars);
  const int maxCars = std::min(20, std::max(1, totalSpots - 1));
  targetCars = std::min(targetCars, maxCars);

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
    if (carOnSpotOk(lg, x, z)) {
      carSpots.push_back({x, z, kYawLeft});
    }
  }
  for (int i = 0; i < nRight; ++i) {
    const float x = spotXRight(i);
    const float z = zRightCenter;
    if (carOnSpotOk(lg, x, z)) {
      carSpots.push_back({x, z, kYawRight});
    }
  }

  const uint64_t seed = layoutHash();
  std::seed_seq seq{static_cast<uint32_t>(seed >> 32u), static_cast<uint32_t>(seed & 0xffffffffu)};
  std::mt19937 rng(seq);
  std::shuffle(carSpots.begin(), carSpots.end(), rng);
  std::uniform_int_distribution<int> carModelDist(0, 1);

  const int placeCount = std::min(targetCars, static_cast<int>(carSpots.size()));
  for (int i = 0; i < placeCount; ++i) {
    const CarSpot& s = carSpots[static_cast<size_t>(i)];
    const uint8_t modelIdx = static_cast<uint8_t>(carModelDist(rng));
    props_.push_back({PropKind::Car, {s.x, kCarY, s.z}, s.yaw, kCarScale, modelIdx});
  }

  constexpr float kLampScale = 0.93f;
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

  constexpr float kRoadLampAlongStepM = 100.0f;
  constexpr float kRoadLampEdgeMargin = 12.0f;
  const float roadXMin = -gHalfL + kRoadLampEdgeMargin;
  const float roadXMax = gHalfL - kRoadLampEdgeMargin;
  const float parkEdgeLeft = -halfL - 4.0f;
  const float parkEdgeRight = halfL + 4.0f;
  for (float x = roadXMin; x < parkEdgeLeft; x += kRoadLampAlongStepM) {
    props_.push_back({PropKind::Lamp, {x, 0.0f, 0.0f}, 0.0f, kLampScale});
  }
  for (float x = parkEdgeRight; x < roadXMax; x += kRoadLampAlongStepM) {
    props_.push_back({PropKind::Lamp, {x, 0.0f, 0.0f}, 3.14159265f, kLampScale});
  }

  const int nAlong = std::max(nLeft, nRight);
  if (nAlong >= kLampEveryNSpots) {
    const float dxAlong = gen.length() / static_cast<float>(nAlong);
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

std::optional<size_t> ParkingScene::pickCarPropIndex(const glm::vec3& rayOrigin, const glm::vec3& rayDir) const {
  const glm::vec3 bmin = -kCarPickHalfExtents;
  const glm::vec3 bmax = kCarPickHalfExtents;
  float bestT = std::numeric_limits<float>::infinity();
  std::optional<size_t> best;
  for (size_t i = 0; i < props_.size(); ++i) {
    if (props_[i].kind != PropKind::Car) {
      continue;
    }
    glm::mat4 m = glm::translate(glm::mat4(1.0f), props_[i].position);
    m = glm::rotate(m, props_[i].rotY, glm::vec3(0.0f, 1.0f, 0.0f));
    m = glm::scale(m, glm::vec3(props_[i].scale));
    const glm::mat4 invM = glm::inverse(m);
    const glm::vec4 oL = invM * glm::vec4(rayOrigin, 1.0f);
    const glm::vec4 dL = invM * glm::vec4(rayDir, 0.0f);
    float t = 0.0f;
    if (raySlabAabb(glm::vec3(oL), glm::vec3(dL), bmin, bmax, t) && t < bestT) {
      bestT = t;
      best = i;
    }
  }
  return best;
}

bool ParkingScene::tryMoveCarToWorldXZ(size_t propIndex, float worldHitX, float worldHitZ) {
  if (propIndex >= props_.size() || props_[propIndex].kind != PropKind::Car) {
    return false;
  }
  LotGeom g{};
  fillLotGeom(generator_, g);
  float sx = 0.0f;
  float sz = 0.0f;
  float yaw = 0.0f;
  if (!snapHitToSpot(g, worldHitX, worldHitZ, sx, sz, yaw)) {
    return false;
  }
  const float sep = std::min(g.dxL, g.dxR) * 0.35f;
  const float sep2 = sep * sep;
  for (size_t i = 0; i < props_.size(); ++i) {
    if (props_[i].kind != PropKind::Car || i == propIndex) {
      continue;
    }
    const float dx = props_[i].position.x - sx;
    const float dz = props_[i].position.z - sz;
    if (dx * dx + dz * dz < sep2) {
      return false;
    }
  }
  props_[propIndex].position.x = sx;
  props_[propIndex].position.z = sz;
  props_[propIndex].position.y = kCarY;
  props_[propIndex].rotY = yaw;
  return true;
}

}  // namespace parking
