#pragma once

#include "lighting/Lighting.hpp"
#include "scene/ParkingGenerator.hpp"

#include <glm/glm.hpp>

#include <cstdint>
#include <vector>

namespace parking {

struct TreeInstance {
  glm::vec3 position{};
  float scale{1.0f};
};

class ParkingScene {
public:
  explicit ParkingScene(ParkingGenerator generator);

  void syncPlacements();

  [[nodiscard]] ParkingGenerator& generator() { return generator_; }
  [[nodiscard]] const ParkingGenerator& generator() const { return generator_; }

  [[nodiscard]] Lighting& lighting() { return lighting_; }
  [[nodiscard]] const Lighting& lighting() const { return lighting_; }

  [[nodiscard]] const std::vector<TreeInstance>& trees() const { return trees_; }

private:
  void rebuildPlacements();
  [[nodiscard]] uint64_t layoutHash() const;

  ParkingGenerator generator_;
  Lighting lighting_;

  std::vector<TreeInstance> trees_;
  uint64_t lastLayout_{0};
};

}  // namespace parking
