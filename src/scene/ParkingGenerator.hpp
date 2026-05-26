#pragma once



namespace parking {



class ParkingGenerator {

public:

  static constexpr float kMinLength = 36.0f;

  static constexpr float kMaxLength = 260.0f;

  static constexpr int kMinSpots = 2;

  static constexpr int kMaxSpots = 360;

  /// Liczba równoległych rzędów miejsc (parametr „szerokości”).
  /// 2 rzędy = jeden pas alejki między nimi (układ wyjściowy).
  /// Każde +1 = +1 alejka + +1 rząd o tej samej liczbie miejsc co sąsiad.
  static constexpr int kMinRows = 2;

  static constexpr int kMaxRows = 6;

  static constexpr int kDefaultRows = 2;

  /// Liczba miejsc na pojedynczy rząd (parametr „długości”).
  static constexpr int kMinSpotsPerRow = 1;

  static constexpr int kMaxSpotsPerRow = 60;

  static constexpr int kDefaultSpotsPerRow = 16;

  /// Pas drogowy między rzędami (oś X — „droga przez trawę” używa tej szerokości w Z).
  static constexpr float aisleWidthMeters() { return 8.5f; }

  /// Stała głębokość pojedynczego rzędu w Z (auto + zapas/krawężnik).
  /// Wybrana tak, aby dwie domyślne kolumny + alejka dawały historyczne 36 m szerokości.
  static constexpr float spotDepthMeters() { return 13.75f; }

  /// Minimalna szerokość miejsca wzdłuż X (m) — szersze stanowiska.
  static constexpr float minSpotWidthAlongLotMeters() { return 4.0f; }

  /// Zewnętrzna plansza trawnika wokół parkingu — duży teren (granice kamery też).
  static constexpr float grassMarginMeters() { return 88.0f; }

  /// Dodatkowy asfalt poza prostokątem miejsc (symetrycznie).
  static constexpr float pavedLotMarginMeters() { return 26.0f; }

  /// Stała historyczna — używana tylko jako wartość poglądowa.
  static constexpr float fixedWidth() {
    return kDefaultRows * spotDepthMeters() + (kDefaultRows - 1) * aisleWidthMeters();
  }



  ParkingGenerator();



  /// Ustawia całkowitą liczbę miejsc; spotsPerRow_ = max(1, n / rowCount_).
  void setSpotCount(int n);

  void setSpotsPerRow(int n);

  void setLength(float lengthMeters);

  /// Zmienia liczbę równoległych rzędów (= „szerokości”); spotsPerRow_ pozostaje, więc nowy rząd ma taką samą liczbę miejsc.
  void setRowCount(int rows);

  /// Ustawia długość parkingu proporcjonalnie do liczby miejsc na rząd (× rozstaw stanowisk).
  void syncLengthToSpotCount();

  [[nodiscard]] int spotCount() const { return spotsPerRow_ * rowCount_; }

  [[nodiscard]] int spotsPerRow() const { return spotsPerRow_; }

  [[nodiscard]] int rowCount() const { return rowCount_; }

  [[nodiscard]] float length() const { return length_; }

  /// Wyliczana szerokość parkingu: R rzędów + (R-1) alejek między nimi.
  [[nodiscard]] float width() const {
    return static_cast<float>(rowCount_) * spotDepthMeters() +
           static_cast<float>(rowCount_ - 1) * aisleWidthMeters();
  }



  /// Dla zachowania zgodności API — wszystkie rzędy mają obecnie tę samą liczbę miejsc.
  [[nodiscard]] int leftRowSpotCount() const { return spotsPerRow_; }

  [[nodiscard]] int rightRowSpotCount() const { return spotsPerRow_; }



  [[nodiscard]] float halfLength() const { return length_ * 0.5f; }

  [[nodiscard]] float halfWidth() const { return width() * 0.5f; }



  [[nodiscard]] float spotLengthAlongLot() const;



private:

  void clampParameters();



  int spotsPerRow_{kDefaultSpotsPerRow};

  int rowCount_{kDefaultRows};

  float length_{72.0f};

};



}  // namespace parking
