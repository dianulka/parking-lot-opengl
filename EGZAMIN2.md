# EGZAMIN 2 — 30 krótkich zadań z rozwiązaniami (`parking-lot-opengl`)

> Zestaw drobnych zadań egzaminacyjnych: każde to **jedna mała zmiana** w kodzie.
> Pod każdym poleceniem znajduje się gotowe rozwiązanie ze wskazaniem pliku i miejsca.
> Numery linii odnoszą się do stanu repozytorium w chwili przygotowania zestawu.

**Legenda kategorii:** `[PARAM]` parametry/stałe · `[LOGIKA]` logika biznesowa ·
`[ZDARZENIE]` reakcja na klawisz/mysz · `[UI]` sposób wyświetlania informacji.

---

## 1. `[PARAM]` Szybsze przesuwanie kamery (WASD)

**Polecenie:** Przyspiesz panoramowanie kamery klawiszami `WASD` dwukrotnie.

**Plik:** `src/app/Application.cpp` (stała `kCameraPanSpeed`).

**Rozwiązanie:**

```cpp
constexpr float kCameraPanSpeed = 76.0f;  // było 38.0f
```

---

## 2. `[PARAM]` Wolniejszy obrót kamery (strzałki)

**Polecenie:** Zmniejsz prędkość obrotu kamery strzałkami o połowę.

**Plik:** `src/app/Application.cpp` (stała `kCameraOrbitSpeed`).

**Rozwiązanie:**

```cpp
constexpr float kCameraOrbitSpeed = 24.0f;  // było 48.0f
```

---

## 3. `[PARAM]` Większy zakres oddalenia kamery

**Polecenie:** Pozwól oddalić kamerę dalej — zmień maksymalny dystans z 420 na 800.

**Plik:** `src/rendering/Camera.cpp`, metoda `zoom`.

**Rozwiązanie:**

```47:51:src/rendering/Camera.cpp
void Camera::zoom(float deltaDistance) {
  distance_ += deltaDistance;
  distance_ = std::clamp(distance_, 8.0f, 420.0f);
  updateEye();
}
```

Zmiana:

```cpp
  distance_ = std::clamp(distance_, 8.0f, 800.0f);
```

---

## 4. `[PARAM]` Inny limit kąta nachylenia kamery (pitch)

**Polecenie:** Pozwól patrzeć bardziej „z boku" — dolny limit pitch z 18° na 5°.

**Plik:** `src/rendering/Camera.cpp`, metoda `orbit`.

**Rozwiązanie:**

```cpp
  pitchDeg_ = std::clamp(pitchDeg_, 5.0f, 88.0f);  // było 18.0f
```

---

## 5. `[ZDARZENIE]` Odwrócenie kierunku scrolla zoomu

**Polecenie:** Odwróć działanie kółka myszy — scroll w górę ma oddalać, w dół przybliżać.

**Plik:** `src/app/Application.cpp`, `scrollCallback`.

**Rozwiązanie:** usuń znak minus:

```219:224:src/app/Application.cpp
void Application::scrollCallback(GLFWwindow* window, double /*xoffset*/, double yoffset) {
  auto* h = static_cast<WindowHooks*>(glfwGetWindowUserPointer(window));
  if (h && h->camera) {
    h->camera->zoom(static_cast<float>(-yoffset) * 1.1f);
  }
}
```

Zmiana:

```cpp
    h->camera->zoom(static_cast<float>(yoffset) * 1.1f);
```

---

## 6. `[PARAM]` Czulszy zoom kółkiem

**Polecenie:** Podwój czułość zoomu kółkiem myszy.

**Plik:** `src/app/Application.cpp`, `scrollCallback`.

**Rozwiązanie:**

```cpp
    h->camera->zoom(static_cast<float>(-yoffset) * 2.2f);  // było 1.1f
```

---

## 7. `[PARAM]` Szersze pole widzenia (FOV)

**Polecenie:** Zmień domyślne pole widzenia kamery z 52° na 70°.

**Plik:** `src/rendering/Camera.hpp` (pole `fovDegrees_`).

**Rozwiązanie:**

```cpp
  float fovDegrees_{70.0f};  // było 52.0f
```

---

## 8. `[PARAM]` Dalsza płaszczyzna odcięcia

**Polecenie:** Zwiększ daleką płaszczyznę odcięcia z 600 na 1500 m.

**Plik:** `src/rendering/Camera.cpp`, metoda `projection`.

**Rozwiązanie:**

```cpp
  return glm::perspective(glm::radians(fovDegrees_), aspect, 0.2f, 1500.0f);
```

---

## 9. `[ZDARZENIE]` Zoom klawiaturą (`+` / `-`)

**Polecenie:** Dodaj możliwość przybliżania klawiszem `=`/`+` i oddalania `-`.

**Plik:** `src/app/Application.cpp`, `updateCamera`.

**Rozwiązanie:** dopisz na końcu `updateCamera`:

```cpp
  const float zoomStep = 40.0f * dt;
  if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) {
    camera.zoom(-zoomStep);
  }
  if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) {
    camera.zoom(zoomStep);
  }
```

---

## 10. `[PARAM]` Domyślnie dzień zamiast nocy

**Polecenie:** Zmień domyślny tryb oświetlenia na dzienny.

**Plik:** `src/lighting/Lighting.hpp` (inicjalizacja `mode_`).

**Rozwiązanie:**

```cpp
  LightingMode mode_{LightingMode::Day};  // było Night
```

---

## 11. `[PARAM]` Inny widok startowy kamery

**Polecenie:** Ustaw startowy obrót kamery na yaw 90° i pitch 45°.

**Plik:** `src/app/Application.cpp`, konstruktor `Application`.

**Rozwiązanie:**

```51:53:src/app/Application.cpp
  constexpr float kStartYawDeg = 39.0f;
  constexpr float kStartPitchDeg = 61.5f;
  camera_.setOrbit(startDist, kStartYawDeg, kStartPitchDeg, glm::vec3(0.0f, 0.0f, 0.0f));
```

Zmiana:

```cpp
  constexpr float kStartYawDeg = 90.0f;
  constexpr float kStartPitchDeg = 45.0f;
```

---

## 12. `[PARAM]` Więcej dozwolonych rzędów

**Polecenie:** Pozwól na maksymalnie 8 rzędów zamiast 6.

**Plik:** `src/scene/ParkingGenerator.hpp` (stała `kMaxRows`).

**Rozwiązanie:**

```cpp
  static constexpr int kMaxRows = 8;  // było 6
```

---

## 13. `[PARAM]` Więcej miejsc na rząd

**Polecenie:** Podnieś limit miejsc na pojedynczy rząd z 60 do 80.

**Plik:** `src/scene/ParkingGenerator.hpp` (stała `kMaxSpotsPerRow`).

**Rozwiązanie:**

```cpp
  static constexpr int kMaxSpotsPerRow = 80;  // było 60
```

---

## 14. `[PARAM]` Wyższy twardy limit miejsc

**Polecenie:** Zwiększ górny limit łącznej liczby miejsc z 360 na 500.

**Plik:** `src/scene/ParkingGenerator.hpp` (stała `kMaxSpots`).

**Rozwiązanie:**

```cpp
  static constexpr int kMaxSpots = 500;  // było 360
```

---

## 15. `[PARAM]` Szersze miejsca postojowe

**Polecenie:** Zwiększ minimalną szerokość miejsca z 4.0 m na 5.0 m.

**Plik:** `src/scene/ParkingGenerator.hpp`, `minSpotWidthAlongLotMeters()`.

**Rozwiązanie:**

```cpp
  static constexpr float minSpotWidthAlongLotMeters() { return 5.0f; }
```

---

## 16. `[PARAM]` Inny rozstaw stanowisk wzdłuż parkingu

**Polecenie:** Zmień rozstaw wyliczanej długości na 5.0 m na miejsce (było 4.5 m).

**Plik:** `src/scene/ParkingGenerator.cpp`, `syncLengthToSpotCount`.

**Rozwiązanie:**

```cpp
  constexpr float kSlotPitchAlongLotMeters = 5.0f;  // było 4.5f
```

---

## 17. `[PARAM]` Szersza alejka między rzędami

**Polecenie:** Poszerz alejkę między rzędami z 8.5 m na 11.0 m.

**Plik:** `src/scene/ParkingGenerator.hpp`, `aisleWidthMeters()`.

**Rozwiązanie:**

```cpp
  static constexpr float aisleWidthMeters() { return 11.0f; }
```

---

## 18. `[PARAM]` Mniejszy trawnik wokół parkingu

**Polecenie:** Zmniejsz margines trawnika (i granice kamery) z 88 m na 50 m.

**Plik:** `src/scene/ParkingGenerator.hpp`, `grassMarginMeters()`.

**Rozwiązanie:**

```cpp
  static constexpr float grassMarginMeters() { return 50.0f; }
```

---

## 19. `[LOGIKA]` Walidacja: odrzuć niedodatnią liczbę miejsc w CLI

**Polecenie:** W argumencie `--spaces`/`-n` odrzuć wartości ≤ 0 z komunikatem błędu i kodem wyjścia 1.

**Plik:** `src/main.cpp`, obsługa `--spaces`.

**Rozwiązanie:** po `parseInt` dodaj sprawdzenie:

```57:64:src/main.cpp
      int n = 0;
      if (!parseInt(argv[++i], n)) {
        std::fprintf(stderr, "Nieprawidlowa liczba miejsc: %s\n", argv[i]);
        return 1;
      }
      gen.setSpotCount(n);
      continue;
```

Zmiana:

```cpp
      int n = 0;
      if (!parseInt(argv[++i], n)) {
        std::fprintf(stderr, "Nieprawidlowa liczba miejsc: %s\n", argv[i]);
        return 1;
      }
      if (n <= 0) {
        std::fprintf(stderr, "Liczba miejsc musi byc dodatnia: %d\n", n);
        return 1;
      }
      gen.setSpotCount(n);
      continue;
```

---

## 20. `[LOGIKA]` Inny procent zapełnienia parkingu

**Polecenie:** Zmień docelowe zapełnienie z ~40% na ~60% miejsc.

**Plik:** `src/scene/ParkingScene.cpp`, `rebuildPlacements`.

**Rozwiązanie:**

```201:201:src/scene/ParkingScene.cpp
  int targetCars = (totalSpots * 2) / 5;
```

Zmiana (60% = `* 3 / 5`):

```cpp
  int targetCars = (totalSpots * 3) / 5;
```

---

## 21. `[LOGIKA]` Wyższy limit liczby aut

**Polecenie:** Pozwól na maksymalnie 40 aut na scenie zamiast 20.

**Plik:** `src/scene/ParkingScene.cpp`, `rebuildPlacements`.

**Rozwiązanie:**

```cpp
  const int maxCars = std::min(40, std::max(1, totalSpots - 1));  // było 20
```

---

## 22. `[LOGIKA]` Zarezerwuj ostatni rząd

**Polecenie:** Zablokuj przenoszenie aut do ostatniego rzędu (najdalszego w Z).

**Plik:** `src/scene/ParkingScene.cpp`, `snapHitToSpot`.

**Rozwiązanie:** po wyborze `bestRow` dodaj warunek:

```cpp
  if (bestRow < 0) {
    return false;
  }
  // Ostatni rząd zarezerwowany — nie pozwalamy tu parkować.
  if (bestRow == static_cast<int>(g.rowCenterZ.size()) - 1) {
    return false;
  }
```

---

## 23. `[LOGIKA]` Większy odstęp między autami

**Polecenie:** Zaostrz kolizję — wymagaj odstępu równego 70% szerokości miejsca (było 35%).

**Plik:** `src/scene/ParkingScene.cpp`, `tryMoveCarToWorldXZ`.

**Rozwiązanie:**

```cpp
  const float sep = g.dx * 0.70f;  // było 0.35f
```

---

## 24. `[LOGIKA]` Łatwiejsze trafianie auta kliknięciem

**Polecenie:** Powiększ obszar „pick" auta — wydłuż pudełko trafień w osi X z 2.15 na 3.0.

**Plik:** `src/scene/ParkingScene.cpp` (stała `kCarPickHalfExtents`).

**Rozwiązanie:**

```22:22:src/scene/ParkingScene.cpp
const glm::vec3 kCarPickHalfExtents(2.15f, 0.52f, 1.0f);
```

Zmiana:

```cpp
const glm::vec3 kCarPickHalfExtents(3.0f, 0.52f, 1.0f);
```

---

## 25. `[LOGIKA]` Dopuść upuszczanie auta także na alejce

**Polecenie:** Zlikwiduj odrzucanie kliknięcia na alejce — auto ma „przyciągać się" do najbliższego rzędu nawet z alejki.

**Plik:** `src/scene/ParkingScene.cpp`, `snapHitToSpot`.

**Rozwiązanie:** usuń (lub zneutralizuj) warunek odrzucający alejkę:

```145:148:src/scene/ParkingScene.cpp
  // Wewnątrz rzędu (a nie alejki).
  if (bestDist > g.spotDepth * 0.5f + 0.01f) {
    return false;
  }
```

Wystarczy usunąć ten `if` (lub zwiększyć próg, np. do `g.spotDepth * 2.0f`, aby objąć alejkę).

---

## 26. `[ZDARZENIE]` Globalne przełączanie dnia/nocy klawiszem `L`

**Polecenie:** Dodaj globalny skrót `L` przełączający dzień/noc niezależnie od panelu.

**Plik:** `src/input/Input.cpp`, `keyCallback` (+ `#include "lighting/Lighting.hpp"`).

**Rozwiązanie:** w `switch` dodaj:

```cpp
    case GLFW_KEY_L: {
      Lighting& lit = h->scene->lighting();
      lit.setMode(lit.mode() == LightingMode::Day ? LightingMode::Night : LightingMode::Day);
      break;
    }
```

---

## 27. `[ZDARZENIE]` Wyjście z programu klawiszem `Q`

**Polecenie:** Dodaj klawisz `Q` zamykający aplikację (oprócz `ESC`).

**Plik:** `src/input/Input.cpp`, `keyCallback`.

**Rozwiązanie:** w `switch` dodaj:

```cpp
    case GLFW_KEY_Q:
      glfwSetWindowShouldClose(window, GLFW_TRUE);
      break;
```

---

## 28. `[ZDARZENIE]` Anulowanie wyboru auta prawym przyciskiem myszy

**Polecenie:** PPM ma anulować zaznaczone auto, nie ruszając panelu ani kamery.

**Plik:** `src/app/Application.cpp`, `mouseButtonCallback`.

**Rozwiązanie:** na początku callbacka (po pobraniu `h`) dodaj:

```cpp
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    h->app->consumeEscapeForCarSelection();
    return;
  }
```

(`consumeEscapeForCarSelection()` jest już publiczne w `Application.hpp`).

---

## 29. `[UI]` Większy przycisk panelu w rogu

**Polecenie:** Powiększ kwadratowy przycisk otwierający panel z 64 px na 96 px.

**Plik:** `src/app/UiConstants.hpp` (stała `kCornerPanelPx`).

**Rozwiązanie:**

```cpp
constexpr float kCornerPanelPx = 96.0f;  // było 64.0f
```

Obszar klikalny w `mouseButtonCallback` korzysta z tej stałej, więc zmiana jest spójna.

---

## 30. `[UI]` Zmień tytuł okna aplikacji

**Polecenie:** Zmień domyślny tytuł okna na `"Symulator Parkingu 3D"`.

**Plik:** `src/app/Application.cpp` (stała `kTitleNormal`).

**Rozwiązanie:**

```25:25:src/app/Application.cpp
constexpr char kTitleNormal[] = "Parking prostokątny — OpenGL 3.3";
```

Zmiana:

```cpp
constexpr char kTitleNormal[] = "Symulator Parkingu 3D";
```

---

## Zadania dodatkowe (bonus)

### B1. `[PARAM]` Inny rozkład modeli aut

**Polecenie:** Spraw, by losowane były tylko modele 1 i 2 (bez AE86, model 0).

**Plik:** `src/scene/ParkingScene.cpp`, `rebuildPlacements`.

**Rozwiązanie:**

```cpp
  std::uniform_int_distribution<int> carModelDist(1, 2);  // było (0, 2)
```

---

### B2. `[PARAM]` Rzadsze lampy na krawędziach

**Polecenie:** Stawiaj dodatkowe lampy co 8 miejsc zamiast co 5.

**Plik:** `src/scene/ParkingScene.cpp`, stała `kLampEveryNSpots`.

**Rozwiązanie:**

```cpp
  constexpr int kLampEveryNSpots = 8;  // było 5
```

---

### B3. `[UI]` Pokaż liczbę aut i lamp w konsoli na starcie

**Polecenie:** Po starcie dopisz w konsoli liczbę zaparkowanych aut i lamp.

**Plik:** `src/app/Application.cpp`, koniec konstruktora.

**Rozwiązanie:** najpierw zsynchronizuj scenę, potem policz i wypisz:

```cpp
  scene_.syncPlacements();
  int cars = 0;
  int lamps = 0;
  for (const auto& p : scene_.props()) {
    (p.kind == PropKind::Car ? cars : lamps)++;
  }
  std::printf("Na scenie: auta=%d, lampy=%d\n", cars, lamps);
```

---

### B4. `[LOGIKA]` Wymuś parzystą liczbę rzędów

**Polecenie:** W `clampParameters` zaokrąglij liczbę rzędów w dół do najbliższej parzystej.

**Plik:** `src/scene/ParkingGenerator.cpp`, `clampParameters`.

**Rozwiązanie:** po istniejącym clampie rzędów dodaj:

```cpp
  rowCount_ = std::clamp(rowCount_, kMinRows, kMaxRows);
  if (rowCount_ % 2 != 0) {
    rowCount_ -= 1;  // parzysta liczba rzędów (symetryczny układ)
  }
  rowCount_ = std::max(rowCount_, kMinRows);
```

---

### B5. `[PARAM]` Auta uniesione nad ziemią

**Polecenie:** Zmień wysokość osadzenia aut z 0.06 na 0.20.

**Plik:** `src/scene/ParkingScene.cpp` (stała `kCarY`).

**Rozwiązanie:**

```cpp
constexpr float kCarY = 0.20f;  // było 0.06f
```

---

## Zadania dodatkowe — ruch kamery

> Kamera to **kamera orbitalna** wokół punktu na ziemi (`target`): `WASD` przesuwa cel po XZ,
> strzałki obracają (yaw/pitch), kółko zmienia dystans. Logika ruchu na klawiaturze jest
> w `Application::updateCamera` (wywoływana co klatkę z `dt`), a same operacje w klasie
> `Camera` (`panWorldXZ`, `orbit`, `zoom`).

### K1. `[ZDARZENIE]` Tryb „sprint" — szybsza kamera z `Shift`

**Polecenie:** Gdy trzymany jest `Left Shift`, przyspiesz całą kamerę (przesuwanie i obrót) 3×.

**Plik:** `src/app/Application.cpp`, `updateCamera`.

**Rozwiązanie:** policz mnożnik na początku funkcji i przeskaluj prędkości:

```271:273:src/app/Application.cpp
void Application::updateCamera(GLFWwindow* window, Camera& camera, float dt) {
  const float pan = kCameraPanSpeed * dt;
  const float orbit = kCameraOrbitSpeed * dt;
```

Zmiana:

```cpp
void Application::updateCamera(GLFWwindow* window, Camera& camera, float dt) {
  const float boost = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ? 3.0f : 1.0f;
  const float pan = kCameraPanSpeed * dt * boost;
  const float orbit = kCameraOrbitSpeed * dt * boost;
```

---

### K2. `[ZDARZENIE]` Reset widoku kamery klawiszem `Home`

**Polecenie:** Klawisz `Home` ma przywracać kamerę do ustawienia startowego (dystans 60, yaw 39°, pitch 61.5°, cel w środku sceny).

**Plik:** `src/app/Application.cpp`, `updateCamera`.

**Rozwiązanie:** dopisz na końcu `updateCamera`:

```cpp
  if (glfwGetKey(window, GLFW_KEY_HOME) == GLFW_PRESS) {
    camera.setOrbit(60.0f, 39.0f, 61.5f, glm::vec3(0.0f, 0.0f, 0.0f));
  }
```

`setOrbit` sam przytnie cel do granic sceny (`clampTarget`) i przeliczy pozycję oka.

---

### K3. `[LOGIKA]` Prędkość przesuwania zależna od oddalenia

**Polecenie:** Gdy kamera jest bardziej oddalona, `WASD` ma przesuwać szybciej (proporcjonalnie do dystansu), aby ruch po dużej mapie nie był wolny.

**Plik:** `src/app/Application.cpp`, `updateCamera`.

**Rozwiązanie:** `Camera` udostępnia `distance()`. Skaluj `pan` względem dystansu startowego (~52):

```cpp
  const float distScale = camera.distance() / 52.0f;
  const float pan = kCameraPanSpeed * dt * distScale;
  const float orbit = kCameraOrbitSpeed * dt;
```

---

### K4. `[ZDARZENIE]` Obrót poziomy (yaw) klawiszami `Q`/`E`

**Polecenie:** Dodaj obrót kamery w poziomie również klawiszami `Q` (w lewo) i `E` (w prawo) — równolegle do strzałek.

**Plik:** `src/app/Application.cpp`, `updateCamera`.

**Rozwiązanie:** dopisz w `updateCamera`:

```cpp
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
    camera.orbit(-orbit, 0.0f);
  }
  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
    camera.orbit(orbit, 0.0f);
  }
```

---

### K5. `[ZDARZENIE]` Zmiana nachylenia (pitch) klawiszami `R`/`F`

**Polecenie:** Dodaj sterowanie kątem nachylenia kamery: `R` unosi widok, `F` opuszcza.

**Plik:** `src/app/Application.cpp`, `updateCamera`.

**Rozwiązanie:** dopisz w `updateCamera`:

```cpp
  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
    camera.orbit(0.0f, orbit * 0.45f);
  }
  if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
    camera.orbit(0.0f, -orbit * 0.45f);
  }
```

(Pitch i tak jest przycinany do zakresu 18°–88° w `Camera::orbit`).

---

### K6. `[LOGIKA]` Przesuwanie względem kierunku patrzenia (view-relative)

**Polecenie:** Obecnie `WASD` przesuwa wzdłuż stałych osi świata. Zmień to tak, aby `W` zawsze
oznaczało „w głąb ekranu" względem aktualnego obrotu kamery (a `A`/`D` w bok). Wymaga to
dodania nowej operacji w klasie `Camera`.

**Pliki:** `src/rendering/Camera.hpp`, `src/rendering/Camera.cpp`, `src/app/Application.cpp`.

**Rozwiązanie:**

1) W `Camera.hpp` dodaj deklarację (obok `panWorldXZ`):

```cpp
  void panRelative(float forward, float right);
```

2) W `Camera.cpp` dodaj definicję — kierunek wyznaczamy z `yawDeg_`:

```cpp
void Camera::panRelative(float forward, float right) {
  const float yaw = glm::radians(yawDeg_);
  // Kierunek "w głąb ekranu" rzutowany na płaszczyznę XZ.
  const float fwdX = -std::sin(yaw);
  const float fwdZ = -std::cos(yaw);
  // Kierunek "w prawo" — prostopadły do powyższego.
  const float rightX = std::cos(yaw);
  const float rightZ = -std::sin(yaw);
  target_.x += forward * fwdX + right * rightX;
  target_.z += forward * fwdZ + right * rightZ;
  clampTarget();
  updateEye();
}
```

3) W `Application::updateCamera` zamień wywołania `panWorldXZ` na `panRelative`:

```cpp
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    camera.panRelative(pan, 0.0f);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    camera.panRelative(-pan, 0.0f);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    camera.panRelative(0.0f, -pan);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    camera.panRelative(0.0f, pan);
  }
```

**Pytanie kontrolne:** dlaczego ruch dotyczy tylko XZ, a `target_.y` zostaje 0? — Bo cel kamery
leży na płaszczyźnie ziemi; `clampTarget()` i tak wymusza `y = 0`.

---

### K7. `[ZDARZENIE]` Odwrócenie pionowego obrotu (oś strzałek góra/dół)

**Polecenie:** Odwróć obrót w pionie — strzałka w górę ma pochylać kamerę w dół i odwrotnie
(tryb „inverted look").

**Plik:** `src/app/Application.cpp`, `updateCamera`.

**Rozwiązanie:** zamień znaki przy obrocie pitch:

```293:298:src/app/Application.cpp
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    camera.orbit(0.0f, orbit * 0.45f);
  }
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    camera.orbit(0.0f, -orbit * 0.45f);
  }
```

Zmiana:

```cpp
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    camera.orbit(0.0f, -orbit * 0.45f);
  }
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    camera.orbit(0.0f, orbit * 0.45f);
  }
```

---

### K8. `[LOGIKA]` Zoom proporcjonalny do dystansu (gładkie zbliżanie)

**Polecenie:** Spraw, by jeden „klik" kółka zmieniał dystans o stały **procent** (np. 10%),
a nie o stałą wartość — dzięki temu zoom jest płynny zarówno z bliska, jak i z daleka.

**Plik:** `src/app/Application.cpp`, `scrollCallback`.

**Rozwiązanie:** użyj `camera.distance()` do wyliczenia kroku proporcjonalnego:

```219:224:src/app/Application.cpp
void Application::scrollCallback(GLFWwindow* window, double /*xoffset*/, double yoffset) {
  auto* h = static_cast<WindowHooks*>(glfwGetWindowUserPointer(window));
  if (h && h->camera) {
    h->camera->zoom(static_cast<float>(-yoffset) * 1.1f);
  }
}
```

Zmiana:

```cpp
void Application::scrollCallback(GLFWwindow* window, double /*xoffset*/, double yoffset) {
  auto* h = static_cast<WindowHooks*>(glfwGetWindowUserPointer(window));
  if (h && h->camera) {
    const float step = h->camera->distance() * 0.10f;  // 10% bieżącego dystansu
    h->camera->zoom(static_cast<float>(-yoffset) * step);
  }
}
```

---

## Tabela skrótowa (plik → co zmieniasz)

| # | Plik | Element |
|---|------|---------|
| 1, 2, 5, 6, 9, 11, 28, 30 | `src/app/Application.cpp` | kamera, scroll, mysz, tytuł |
| K1–K5, K7, K8 | `src/app/Application.cpp` | ruch kamery (sprint, reset, zoom, klawisze) |
| K6 | `src/rendering/Camera.*` + `Application.cpp` | przesuwanie względem widoku |
| 3, 4, 7, 8 | `src/rendering/Camera.*` | zoom, pitch, FOV, far |
| 10 | `src/lighting/Lighting.hpp` | tryb domyślny |
| 12–18 | `src/scene/ParkingGenerator.*` | limity i wymiary |
| 19 | `src/main.cpp` | walidacja CLI |
| 20–25, B1, B2, B5 | `src/scene/ParkingScene.cpp` | logika sceny/aut/lamp |
| 26, 27 | `src/input/Input.cpp` | globalne klawisze |
| 29 | `src/app/UiConstants.hpp` | przycisk panelu |
| B4 | `src/scene/ParkingGenerator.cpp` | walidacja rzędów |
