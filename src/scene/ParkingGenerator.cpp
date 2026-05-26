#include "scene/ParkingGenerator.hpp"

#include <algorithm>
#include <cmath>

namespace parking {

ParkingGenerator::ParkingGenerator() { clampParameters(); }

void ParkingGenerator::clampParameters() {
  rowCount_ = std::clamp(rowCount_, kMinRows, kMaxRows);
  spotsPerRow_ = std::clamp(spotsPerRow_, kMinSpotsPerRow, kMaxSpotsPerRow);
  length_ = std::clamp(length_, kMinLength, kMaxLength);

  // Ograniczenie z geometrii: spotsPerRow * minSpotWidth nie może przekroczyć długości.
  const int maxForLength =
      static_cast<int>(std::floor(length_ / ParkingGenerator::minSpotWidthAlongLotMeters()));
  if (maxForLength >= kMinSpotsPerRow && spotsPerRow_ > maxForLength) {
    spotsPerRow_ = maxForLength;
  }

  // Twardy limit na łączną liczbę miejsc — chroni przed scenami z setkami aut.
  const int total = spotsPerRow_ * rowCount_;
  if (total > kMaxSpots) {
    spotsPerRow_ = std::max(kMinSpotsPerRow, kMaxSpots / rowCount_);
  }
}

void ParkingGenerator::setSpotCount(int n) {
  spotsPerRow_ = std::max(1, n / std::max(1, rowCount_));
  clampParameters();
}

void ParkingGenerator::setSpotsPerRow(int n) {
  spotsPerRow_ = n;
  clampParameters();
}

void ParkingGenerator::setLength(float lengthMeters) {
  length_ = lengthMeters;
  clampParameters();
}

void ParkingGenerator::setRowCount(int rows) {
  rowCount_ = rows;
  clampParameters();
}

void ParkingGenerator::syncLengthToSpotCount() {
  clampParameters();
  const int perRow = std::max(1, spotsPerRow_);
  constexpr float kSlotPitchAlongLotMeters = 4.5f;
  float L = static_cast<float>(perRow) * kSlotPitchAlongLotMeters;
  L = std::clamp(L, kMinLength, kMaxLength);
  length_ = L;
  clampParameters();
}

float ParkingGenerator::spotLengthAlongLot() const {
  const int perRow = std::max(1, spotsPerRow_);
  return length_ / static_cast<float>(perRow);
}

}  // namespace parking
