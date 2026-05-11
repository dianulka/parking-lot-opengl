#pragma once

#include "lighting/Lighting.hpp"
#include "scene/ParkingGenerator.hpp"

#include <glm/glm.hpp>

#include <cstdint>
#include <optional>
#include <vector>

namespace parking {

enum class PropKind : uint8_t { Car, Lamp };

struct PlacedProp {
  PropKind kind{PropKind::Lamp};
  glm::vec3 position{};
  float rotY{0.0f};
  float scale{1.0f};
  /// Dla `Car`: 0 = AE86, 1 = GR86/Xeno GT, 2 = Avalon Hybrid Limited. Dla lamp ignorowane.
  uint8_t carModel{0};
};

class ParkingScene {
public:
  explicit ParkingScene(ParkingGenerator generator);

  void syncPlacements();

  [[nodiscard]] ParkingGenerator& generator() { return generator_; }
  [[nodiscard]] const ParkingGenerator& generator() const { return generator_; }

  [[nodiscard]] Lighting& lighting() { return lighting_; }
  [[nodiscard]] const Lighting& lighting() const { return lighting_; }

  [[nodiscard]] const std::vector<PlacedProp>& props() const { return props_; }

  /// Najbliższy trafiony samochód wzdłuż promienia (tylko `PropKind::Car`).
  [[nodiscard]] std::optional<size_t> pickCarPropIndex(const glm::vec3& rayOrigin, const glm::vec3& rayDir) const;

  /// Przenosi auto na najbliższe legalne miejsce przy (worldHitX, worldHitZ); false przy kolizji / jezdni / braku
  /// indeksu.
  bool tryMoveCarToWorldXZ(size_t propIndex, float worldHitX, float worldHitZ);

private:
  void rebuildPlacements();
  [[nodiscard]] uint64_t layoutHash() const;

  ParkingGenerator generator_;
  Lighting lighting_;

  std::vector<PlacedProp> props_;
  uint64_t lastLayout_{0};
};

}  // namespace parking
