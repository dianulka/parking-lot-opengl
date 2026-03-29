#include "scene/ParkingGenerator.hpp"

#include <algorithm>
#include <cmath>

namespace parking {

ParkingGenerator::ParkingGenerator() { clampParameters(); }

void ParkingGenerator::clampParameters() {
  spotCount_ = std::clamp(spotCount_, kMinSpots, kMaxSpots);
  length_ = std::clamp(length_, kMinLength, kMaxLength);

  const int maxForLength =
      static_cast<int>(std::floor(length_ / ParkingGenerator::minSpotWidthAlongLotMeters())) * 2;
  if (maxForLength >= kMinSpots && spotCount_ > maxForLength) {
    spotCount_ = maxForLength;
  }
  if (spotCount_ > kMaxSpots) {
    spotCount_ = kMaxSpots;
  }
}

void ParkingGenerator::setSpotCount(int n) {
  spotCount_ = n;
  clampParameters();
}

void ParkingGenerator::setLength(float lengthMeters) {
  length_ = lengthMeters;
  clampParameters();
}

int ParkingGenerator::spotsPerRow() const {
  return (spotCount_ + 1) / 2;
}

float ParkingGenerator::spotLengthAlongLot() const {
  const int perRow = std::max(1, spotsPerRow());
  return length_ / static_cast<float>(perRow);
}

}  // namespace parking
