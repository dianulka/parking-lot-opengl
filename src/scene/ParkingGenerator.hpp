#pragma once



namespace parking {



class ParkingGenerator {

public:

  static constexpr float kMinLength = 36.0f;

  static constexpr float kMaxLength = 260.0f;

  static constexpr int kMinSpots = 2;

  static constexpr int kMaxSpots = 120;



  /// Szerokość płyty: dwa rzędy miejsc + pas przejazdu (powiększone, lepszy układ względem tekstury).

  static constexpr float fixedWidth() { return 36.0f; }



  /// Pas drogowy między rzędami (oś X — „droga przez trawę” używa tej szerokości w Z).

  static constexpr float aisleWidthMeters() { return 8.5f; }



  /// Minimalna szerokość miejsca wzdłuż X (m) — szersze stanowiska.

  static constexpr float minSpotWidthAlongLotMeters() { return 4.0f; }



  /// Zewnętrzna plansza trawnika wokół parkingu — duży teren (granice kamery też).

  static constexpr float grassMarginMeters() { return 88.0f; }



  /// Dodatkowy asfalt poza prostokątem miejsc (symetrycznie).

  static constexpr float pavedLotMarginMeters() { return 26.0f; }



  ParkingGenerator();



  void setSpotCount(int n);

  void setLength(float lengthMeters);

  /// Ustawia długość parkingu proporcjonalnie do bieżącej liczby miejsc (rząd z większą liczbą stanowisk × szerokość stanowiska).
  void syncLengthToSpotCount();

  [[nodiscard]] int spotCount() const { return spotCount_; }

  [[nodiscard]] float length() const { return length_; }



  [[nodiscard]] int spotsPerRow() const;



  [[nodiscard]] int leftRowSpotCount() const { return (spotCount_ + 1) / 2; }

  [[nodiscard]] int rightRowSpotCount() const { return spotCount_ / 2; }



  [[nodiscard]] float halfLength() const { return length_ * 0.5f; }

  [[nodiscard]] float halfWidth() const { return fixedWidth() * 0.5f; }



  [[nodiscard]] float spotLengthAlongLot() const;



private:

  void clampParameters();



  int spotCount_{32};

  float length_{72.0f};

};



}  // namespace parking


