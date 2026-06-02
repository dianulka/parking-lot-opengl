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

/// Geometria parkingu z obsługą R równoległych rzędów (każde +1 do rowCount = +1 alejka + +1 rząd).
struct LotGeom {
  float L{};
  float halfL{};
  float halfW{};
  float halfA{};                       // half-szerokość pojedynczej alejki
  float spotDepth{};                   // głębokość rzędu w Z
  int rowCount{};
  int spotsPerRow{};
  float dx{};                          // krok w X między miejscami (jednakowy dla każdego rzędu)
  std::vector<float> rowCenterZ;       // środki rzędów w Z (rozmiar = rowCount)
  std::vector<float> aisleCenterZ;     // środki alejek (rozmiar = rowCount - 1)
  std::vector<float> rowYaw;           // yaw aut w danym rzędzie
  float grassL{};
  float gHalfL{};
};

void fillLotGeom(const ParkingGenerator& gen, LotGeom& g) {
  g.L = gen.length();
  const float W = gen.width();
  const float aisle = ParkingGenerator::aisleWidthMeters();
  const float mg = ParkingGenerator::grassMarginMeters();
  g.halfL = g.L * 0.5f;
  g.halfW = W * 0.5f;
  g.halfA = aisle * 0.5f;
  g.spotDepth = ParkingGenerator::spotDepthMeters();
  g.rowCount = std::max(2, gen.rowCount());
  g.spotsPerRow = std::max(1, gen.spotsPerRow());
  g.dx = g.L / static_cast<float>(g.spotsPerRow);
  g.grassL = g.L + 2.0f * mg;
  g.gHalfL = g.grassL * 0.5f;

  g.rowCenterZ.clear();
  g.rowCenterZ.reserve(static_cast<size_t>(g.rowCount));
  const float step = g.spotDepth + aisle;
  const float startZ = -g.halfW + g.spotDepth * 0.5f;
  for (int k = 0; k < g.rowCount; ++k) {
    g.rowCenterZ.push_back(startZ + static_cast<float>(k) * step);
  }
  g.aisleCenterZ.clear();
  g.aisleCenterZ.reserve(static_cast<size_t>(std::max(0, g.rowCount - 1)));
  for (int k = 0; k < g.rowCount - 1; ++k) {
    g.aisleCenterZ.push_back(0.5f * (g.rowCenterZ[k] + g.rowCenterZ[k + 1]));
  }

  // Yaw aut: "tyłem do najbliższej alejki" — skrajne rzędy jednoznacznie, środkowe rzędy orientujemy
  // wg znaku zc (z < 0 jak stary lewy rząd, z >= 0 jak prawy).
  constexpr float kPi = 3.14159265f;
  g.rowYaw.clear();
  g.rowYaw.reserve(static_cast<size_t>(g.rowCount));
  for (int k = 0; k < g.rowCount; ++k) {
    if (k == 0) {
      g.rowYaw.push_back(0.0f);
    } else if (k == g.rowCount - 1) {
      g.rowYaw.push_back(kPi);
    } else {
      g.rowYaw.push_back(g.rowCenterZ[static_cast<size_t>(k)] < 0.0f ? 0.0f : kPi);
    }
  }
}

bool carOnSpotOk(const LotGeom& g, float x, float z) {
  if (std::abs(x) > g.halfL + 0.01f) {
    return false;
  }
  const float halfDepth = g.spotDepth * 0.5f + 0.01f;
  for (float zc : g.rowCenterZ) {
    if (std::abs(z - zc) <= halfDepth) {
      return true;
    }
  }
  return false;
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
  if (g.rowCenterZ.empty()) {
    return false;
  }
  // Wybór najbliższego rzędu po Z; odrzucamy klik na alejce (przerwa między rzędami).
  int bestRow = -1;
  float bestDist = std::numeric_limits<float>::infinity();
  for (int k = 0; k < static_cast<int>(g.rowCenterZ.size()); ++k) {
    const float d = std::abs(hitZ - g.rowCenterZ[static_cast<size_t>(k)]);
    if (d < bestDist) {
      bestDist = d;
      bestRow = k;
    }
  }
  if (bestRow < 0) {
    return false;
  }
  if (bestRow == 0) {
    return false;
  }
  // Wewnątrz rzędu (a nie alejki).
  if (bestDist > g.spotDepth * 0.5f + 0.01f) {
    return false;
  }
  const float dx = g.dx;
  const float t = (hitX + g.halfL) / dx - 0.5f;
  int idx = static_cast<int>(std::lround(t));
  idx = std::clamp(idx, 0, g.spotsPerRow - 1);
  outX = -g.halfL + (static_cast<float>(idx) + 0.5f) * dx;
  outZ = g.rowCenterZ[static_cast<size_t>(bestRow)];
  outYaw = g.rowYaw[static_cast<size_t>(bestRow)];
  return carOnSpotOk(g, outX, outZ);
}

}  // namespace

ParkingScene::ParkingScene(ParkingGenerator generator) : generator_(std::move(generator)) {}

uint64_t ParkingScene::layoutHash() const {
  const uint32_t lenBits = static_cast<uint32_t>(generator_.length() * 1000.0f);
  const uint32_t rows = static_cast<uint32_t>(generator_.rowCount());
  const uint32_t perRow = static_cast<uint32_t>(generator_.spotsPerRow());
  constexpr uint32_t kPlacementVer = 22u;
  const uint32_t lo = perRow ^ (rows * 0x9E3779B1u) ^ (kPlacementVer * 0x85EBCA77u);
  return (static_cast<uint64_t>(lenBits) << 32) | static_cast<uint64_t>(lo);
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
  const float gHalfL = lg.gHalfL;
  const float dx = lg.dx;
  const int spotsPerRow = lg.spotsPerRow;
  const int rowCount = lg.rowCount;

  auto spotXAt = [&](int idx) { return -halfL + (static_cast<float>(idx) + 0.5f) * dx; };

  constexpr float kCarScale = 1.0f;

  const int totalSpots = spotsPerRow * rowCount;
  // Cel: ok. 40% miejsc zajętych, ale nie więcej niż 20 aut (limity sceny).
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
  for (int k = 0; k < rowCount; ++k) {
    const float z = lg.rowCenterZ[static_cast<size_t>(k)];
    const float yaw = lg.rowYaw[static_cast<size_t>(k)];
    for (int i = 0; i < spotsPerRow; ++i) {
      const float x = spotXAt(i);
      if (carOnSpotOk(lg, x, z)) {
        carSpots.push_back({x, z, yaw});
      }
    }
  }

  const uint64_t seed = layoutHash();
  std::seed_seq seq{static_cast<uint32_t>(seed >> 32u), static_cast<uint32_t>(seed & 0xffffffffu)};
  std::mt19937 rng(seq);
  std::shuffle(carSpots.begin(), carSpots.end(), rng);
  std::uniform_int_distribution<int> carModelDist(0, 2);

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

  // Lampy na 4 narożnikach parkingu.
  props_.push_back({PropKind::Lamp, {-halfL + inset, 0.0f, -halfW + inset}, 0.65f + kPi, kLampScale});
  props_.push_back({PropKind::Lamp, {halfL - inset, 0.0f, -halfW + inset}, -0.65f + kPi, kLampScale});
  props_.push_back({PropKind::Lamp, {-halfL + inset, 0.0f, halfW - inset}, -0.65f, kLampScale});
  props_.push_back({PropKind::Lamp, {halfL - inset, 0.0f, halfW - inset}, 0.65f, kLampScale});

  // Lampy na końcach każdej alejki (po lewej i prawej krawędzi parkingu).
  for (float zAisle : lg.aisleCenterZ) {
    props_.push_back({PropKind::Lamp, {-gHalfL * kRoadEndX, 0.0f, zAisle}, 0.0f, kLampScale});
    props_.push_back({PropKind::Lamp, {gHalfL * kRoadEndX, 0.0f, zAisle}, kPi, kLampScale});
  }

  // Lampy wzdłuż drogi dojazdowej do parkingu (poza obrysem) — przebieg po Z=0 jak dotąd.
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
    props_.push_back({PropKind::Lamp, {x, 0.0f, 0.0f}, kPi, kLampScale});
  }

  // Dodatkowe lampy co kLampEveryNSpots na zewnętrznych krawędziach parkingu (góra/dół po Z).
  if (spotsPerRow >= kLampEveryNSpots) {
    const float dxAlong = gen.length() / static_cast<float>(spotsPerRow);
    auto xAlong = [&](int i) { return -halfL + (static_cast<float>(i) + 0.5f) * dxAlong; };
    constexpr int kMaxExtraPairs = 21;
    int pairs = 0;
    for (int i = kLampEveryNSpots - 1; i < spotsPerRow && pairs < kMaxExtraPairs; i += kLampEveryNSpots) {
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
  const float sep = g.dx * 0.35f;
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
