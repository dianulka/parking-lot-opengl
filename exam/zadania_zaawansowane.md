# 🔧 Zadania zaawansowane — zmiany logiki i interakcji

> [!IMPORTANT]
> Wszystkie zadania skupiają się na **logice** — nowe interakcje, mechaniki, kamera, auta, UI.
> Każde zadanie: **pytanie prowadzącego → pliki → pełny kod → wyjaśnienie**.

---

## 🎥 KAMERA — obrót, zoom, animacje

---

### Zadanie 1: „Dodaj automatyczny obrót kamery wokół parkingu (po naciśnięciu klawisza O kamera obraca się non-stop)"

**Pliki**: [Application.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.hpp), [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp), [Input.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/input/Input.cpp)

**Krok 1** — Dodaj flagę w [Application.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.hpp), po linii `bool prevKeyN_{false};` (~55):
```cpp
  bool autoRotate_{false};
```

**Krok 2** — Dodaj klawisz toggle w [Input.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/input/Input.cpp), w `switch(key)`:
```cpp
    case GLFW_KEY_O:
      if (h->app) {
        h->app->autoRotate_ = !h->app->autoRotate_;
      }
      break;
```

> [!NOTE]
> Pole `autoRotate_` jest `private`, więc albo zrób je `public`, albo dodaj metodę `toggleAutoRotate()` w `Application` i wywołaj ją z `Input`.

Prostsza opcja — zmień w [Application.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.hpp):
```diff
- bool autoRotate_{false};
+ public: bool autoRotate_{false}; private:
```

Lub lepiej — dodaj metodę:
```cpp
  // W Application.hpp, w sekcji public:
  void toggleAutoRotate() { autoRotate_ = !autoRotate_; }
```
I w `Input.cpp`:
```cpp
    case GLFW_KEY_O:
      if (h->app) { h->app->toggleAutoRotate(); }
      break;
```

**Krok 3** — W [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp) `run()`, po linii `updateCamera(window_.native(), camera_, dt);` (~309):
```cpp
    if (autoRotate_) {
      camera_.orbit(25.0f * dt, 0.0f);  // 25°/s obrotu
    }
```

**Wyjaśnienie**: `camera_.orbit(deltaYaw, deltaPitch)` dodaje obrót. Mnożymy przez `dt` żeby prędkość była niezależna od FPS.

---

### Zadanie 2: „Dodaj płynne przybliżanie i oddalanie kamery klawiszami + i - (nie scrollem)"

**Plik**: [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp), w metodzie `updateCamera()` (~271-298)

Dodaj na końcu metody `updateCamera`, przed zamykającym `}`:
```cpp
  if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS ||
      glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
    camera.zoom(-20.0f * dt);  // ujemna wartość = przybliżenie
  }
  if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS ||
      glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
    camera.zoom(20.0f * dt);   // dodatnia = oddalenie
  }
```

**Wyjaśnienie**: `camera.zoom(delta)` zmienia `distance_`. Wartość ujemna przybliża (mniejsza odległość), dodatnia oddala. Mnożymy przez `dt` (delta time) żeby ruch był płynny i niezależny od FPS. `GLFW_KEY_EQUAL` to klawisz `=`/`+` na klawiaturze zwykłej, `KP_ADD/SUBTRACT` to numpad.

---

### Zadanie 3: „Dodaj klawisz T do przełączania między widokiem perspektywicznym a widokiem z góry (top-down)"

**Pliki**: [Application.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.hpp), [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp), [Input.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/input/Input.cpp)

**Krok 1** — W [Application.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.hpp), dodaj pola:
```cpp
  bool topDownView_{false};
  float savedYaw_{39.0f};
  float savedPitch_{61.5f};
  float savedDistance_{52.0f};
```

Dodaj publiczną metodę:
```cpp
  void toggleTopDown();
```

**Krok 2** — W [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp), dodaj implementację:
```cpp
void Application::toggleTopDown() {
  if (!topDownView_) {
    // Zapisz aktualny widok
    savedYaw_ = 0.0f;      // nie mamy gettera, więc resetujemy do 0
    savedPitch_ = 61.5f;
    savedDistance_ = camera_.distance();
    // Ustaw top-down
    camera_.setOrbit(camera_.distance(), 0.0f, 87.0f, glm::vec3(0.0f));
    topDownView_ = true;
  } else {
    // Przywróć poprzedni widok
    camera_.setOrbit(savedDistance_, 39.0f, 61.5f, glm::vec3(0.0f));
    topDownView_ = false;
  }
}
```

**Krok 3** — W [Input.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/input/Input.cpp):
```cpp
    case GLFW_KEY_T:
      if (h->app) { h->app->toggleTopDown(); }
      break;
```

---

### Zadanie 4: „Dodaj animację kamery — po naciśnięciu C kamera płynnie przelatuje do widoku z góry (interpolacja)"

**Pliki**: [Application.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.hpp), [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp)

**Krok 1** — W [Application.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.hpp):
```cpp
  // Animacja kamery
  bool cameraAnimating_{false};
  float animProgress_{0.0f};
  float animStartYaw_{0.0f};
  float animStartPitch_{0.0f};
  float animStartDist_{0.0f};
  float animTargetYaw_{0.0f};
  float animTargetPitch_{87.0f};
  float animTargetDist_{80.0f};
public:
  void startCameraAnimation();
```

**Krok 2** — W [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp):
```cpp
void Application::startCameraAnimation() {
  cameraAnimating_ = true;
  animProgress_ = 0.0f;
  // Startowe wartości = bieżące (tutaj uproszczone)
  animStartYaw_ = 39.0f;
  animStartPitch_ = 61.5f;
  animStartDist_ = camera_.distance();
  animTargetYaw_ = 0.0f;
  animTargetPitch_ = 87.0f;
  animTargetDist_ = camera_.distance();
}
```

**Krok 3** — W `Application::run()`, po `updateCamera(...)`:
```cpp
    if (cameraAnimating_) {
      animProgress_ += dt * 0.8f;  // 0.8 = prędkość animacji (1.25 sekundy)
      if (animProgress_ >= 1.0f) {
        animProgress_ = 1.0f;
        cameraAnimating_ = false;
      }
      // Smooth interpolation (ease-in-out)
      float t = animProgress_;
      t = t * t * (3.0f - 2.0f * t);  // smoothstep
      float yaw = animStartYaw_ + (animTargetYaw_ - animStartYaw_) * t;
      float pitch = animStartPitch_ + (animTargetPitch_ - animStartPitch_) * t;
      float dist = animStartDist_ + (animTargetDist_ - animStartDist_) * t;
      camera_.setOrbit(dist, yaw, pitch, glm::vec3(0.0f));
    }
```

**Krok 4** — W `Input.cpp`:
```cpp
    case GLFW_KEY_C:
      if (h->app) { h->app->startCameraAnimation(); }
      break;
```

---

### Zadanie 5: „Dodaj klawisz Z/X do obracania kamery w lewo/prawo po 45° na jedno naciśnięcie"

**Plik**: [Input.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/input/Input.cpp)

Dodaj include na górze:
```cpp
#include "rendering/Camera.hpp"
```

W `switch(key)`:
```cpp
    case GLFW_KEY_Z:
      if (h->camera) { h->camera->orbit(-45.0f, 0.0f); }
      break;
    case GLFW_KEY_X:
      if (h->camera) { h->camera->orbit(45.0f, 0.0f); }
      break;
```

**Wyjaśnienie**: `orbit(deltaYaw, deltaPitch)` — przy jednorazowym wciśnięciu (nie trzymaniu) obraca o dokładnie 45°. Minusowy yaw = obrót w lewo, plusowy = w prawo.

---

### Zadanie 6: „Dodaj obsługę obracania kamery myszą (prawy przycisk + przeciąganie)"

**Pliki**: [Application.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.hpp), [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp)

**Krok 1** — W [Application.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.hpp), dodaj pola:
```cpp
  bool rightMouseDrag_{false};
  double lastMouseX_{0.0};
  double lastMouseY_{0.0};
```

**Krok 2** — Dodaj callback w [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp).

Zmień `mouseButtonCallback` — dodaj obsługę prawego przycisku:
```cpp
void Application::mouseButtonCallback(GLFWwindow* window, int button, int action, int /*mods*/) {
  auto* h = static_cast<WindowHooks*>(glfwGetWindowUserPointer(window));
  if (!h || !h->app) return;

  // Prawy przycisk — orbita myszą
  if (button == GLFW_MOUSE_BUTTON_RIGHT) {
    if (action == GLFW_PRESS) {
      h->app->rightMouseDrag_ = true;
      glfwGetCursorPos(window, &h->app->lastMouseX_, &h->app->lastMouseY_);
    } else if (action == GLFW_RELEASE) {
      h->app->rightMouseDrag_ = false;
    }
    return;
  }

  // Lewy przycisk — istniejący kod...
  if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS) {
    return;
  }
  // ... reszta istniejącego kodu mouseButtonCallback ...
```

**Krok 3** — W `Application::run()`, po `updateCamera(...)`:
```cpp
    if (rightMouseDrag_) {
      double mx, my;
      glfwGetCursorPos(window_.native(), &mx, &my);
      float dx = static_cast<float>(mx - lastMouseX_) * 0.3f;
      float dy = static_cast<float>(my - lastMouseY_) * 0.2f;
      camera_.orbit(dx, -dy);
      lastMouseX_ = mx;
      lastMouseY_ = my;
    }
```

---

## 🚗 SAMOCHODY — zaznaczanie, przenoszenie, logika

---

### Zadanie 7: „Dodaj możliwość usunięcia samochodu z parkingu (klik na auto + klawisz Delete)"

**Pliki**: [Application.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.hpp), [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp), [ParkingScene.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingScene.hpp), [ParkingScene.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingScene.cpp), [Input.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/input/Input.cpp)

**Krok 1** — W [ParkingScene.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingScene.hpp), dodaj metodę publiczną:
```cpp
  void removeCarProp(size_t propIndex);
```

**Krok 2** — W [ParkingScene.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingScene.cpp):
```cpp
void ParkingScene::removeCarProp(size_t propIndex) {
  if (propIndex < props_.size() && props_[propIndex].kind == PropKind::Car) {
    props_.erase(props_.begin() + static_cast<std::ptrdiff_t>(propIndex));
  }
}
```

**Krok 3** — W [Application.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.hpp), dodaj publiczną metodę:
```cpp
  void deleteSelectedCar();
```

**Krok 4** — W [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp):
```cpp
void Application::deleteSelectedCar() {
  if (!selectedCarPropIndex_.has_value()) return;
  scene_.removeCarProp(*selectedCarPropIndex_);
  selectedCarPropIndex_.reset();
}
```

**Krok 5** — W [Input.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/input/Input.cpp):
```cpp
    case GLFW_KEY_DELETE:
    case GLFW_KEY_BACKSPACE:
      if (h->app) { h->app->deleteSelectedCar(); }
      break;
```

---

### Zadanie 8: „Dodaj klawisz P do zaparkowania auta na losowym wolnym miejscu"

**Pliki**: [ParkingScene.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingScene.hpp), [ParkingScene.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingScene.cpp), [Input.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/input/Input.cpp)

**Krok 1** — W [ParkingScene.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingScene.hpp):
```cpp
  /// Dodaje nowy samochód na losowym wolnym miejscu. Zwraca true jeśli znalazł miejsce.
  bool addCarToRandomFreeSpot();
```

**Krok 2** — W [ParkingScene.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingScene.cpp):
```cpp
bool ParkingScene::addCarToRandomFreeSpot() {
  LotGeom g{};
  fillLotGeom(generator_, g);

  // Zbierz wszystkie miejsca
  std::vector<std::tuple<float, float, float>> freeSpots;  // x, z, yaw
  for (int k = 0; k < g.rowCount; ++k) {
    const float z = g.rowCenterZ[static_cast<size_t>(k)];
    const float yaw = g.rowYaw[static_cast<size_t>(k)];
    for (int i = 0; i < g.spotsPerRow; ++i) {
      const float x = -g.halfL + (static_cast<float>(i) + 0.5f) * g.dx;
      // Sprawdź czy wolne
      bool occupied = false;
      const float sep = g.dx * 0.35f;
      for (const auto& p : props_) {
        if (p.kind != PropKind::Car) continue;
        float ddx = p.position.x - x;
        float ddz = p.position.z - z;
        if (ddx * ddx + ddz * ddz < sep * sep) { occupied = true; break; }
      }
      if (!occupied) {
        freeSpots.emplace_back(x, z, yaw);
      }
    }
  }

  if (freeSpots.empty()) return false;

  // Losowe miejsce
  std::mt19937 rng(static_cast<unsigned>(std::chrono::steady_clock::now().time_since_epoch().count()));
  std::uniform_int_distribution<size_t> dist(0, freeSpots.size() - 1);
  auto [x, z, yaw] = freeSpots[dist(rng)];
  std::uniform_int_distribution<int> modelDist(0, 2);
  uint8_t model = static_cast<uint8_t>(modelDist(rng));

  props_.push_back({PropKind::Car, {x, 0.06f, z}, yaw, 1.0f, model});
  return true;
}
```

Dodaj include na górze ParkingScene.cpp:
```cpp
#include <chrono>
```

**Krok 3** — W [Input.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/input/Input.cpp):
```cpp
    case GLFW_KEY_P:
      if (h->scene) {
        h->scene->addCarToRandomFreeSpot();
      }
      break;
```

---

### Zadanie 9: „Dodaj klawisz G do zaparkowania samochodów na WSZYSTKICH wolnych miejscach naraz"

Wykorzystuje metodę z Zadania 8. W [Input.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/input/Input.cpp):
```cpp
    case GLFW_KEY_G:
      if (h->scene) {
        // Dodawaj auta dopóki są wolne miejsca
        while (h->scene->addCarToRandomFreeSpot()) {}
      }
      break;
```

---

### Zadanie 10: „Dodaj klawisz U do usunięcia WSZYSTKICH samochodów z parkingu"

**Plik**: [ParkingScene.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingScene.hpp) — dodaj metodę:
```cpp
  void removeAllCars();
```

**Plik**: [ParkingScene.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingScene.cpp):
```cpp
void ParkingScene::removeAllCars() {
  props_.erase(
    std::remove_if(props_.begin(), props_.end(),
                   [](const PlacedProp& p) { return p.kind == PropKind::Car; }),
    props_.end());
}
```

**Plik**: [Input.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/input/Input.cpp):
```cpp
    case GLFW_KEY_U:
      if (h->scene) { h->scene->removeAllCars(); }
      break;
```

---

### Zadanie 11: „Dodaj licznik samochodów i wolnych miejsc wyświetlany w tytule okna"

**Plik**: [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp)

Zmień metodę `updateWindowTitle()`:
```cpp
void Application::updateWindowTitle() {
  // Policz auta
  int carCount = 0;
  for (const auto& p : scene_.props()) {
    if (p.kind == PropKind::Car) ++carCount;
  }
  const int total = scene_.generator().spotCount();
  const int free = total - carCount;

  if (!settingsOpen_) {
    char buf[200];
    std::snprintf(buf, sizeof(buf),
                  "Parking — Auta: %d/%d | Wolne: %d | OpenGL 3.3",
                  carCount, total, free);
    glfwSetWindowTitle(window_.native(), buf);
    return;
  }
  // ... reszta istniejącego kodu dla settingsOpen_ ...
```

---

### Zadanie 12: „Zrób tak, żeby nowo zaznaczone auto zmieniało kolor (podświetlenie wybranego auta)"

**Plik**: [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp)

W głównej pętli rysowania modeli (~993-1015), zmień `draw()`:

Najpierw musisz przekazać `selectedCarPropIndex` do Renderer. W metodzie `draw()` masz już parametr `carAwaitingDestination`. Dodaj nowy parametr — albo przenieś logikę do Renderer.

Prostsze rozwiązanie — przekaż indeks zaznaczonego auta. Zmień sygnaturę `draw()`:

W [Renderer.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.hpp):
```diff
- void draw(ParkingScene& scene, Camera& camera, float timeSec, bool parkingSettingsOpen,
-           bool carAwaitingDestination);
+ void draw(ParkingScene& scene, Camera& camera, float timeSec, bool parkingSettingsOpen,
+           bool carAwaitingDestination, int selectedPropIndex = -1);
```

W [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp) `run()`:
```diff
- renderer_.draw(scene_, camera_, static_cast<float>(now), settingsOpen_, selectedCarPropIndex_.has_value());
+ int selIdx = selectedCarPropIndex_.has_value() ? static_cast<int>(*selectedCarPropIndex_) : -1;
+ renderer_.draw(scene_, camera_, static_cast<float>(now), settingsOpen_, selectedCarPropIndex_.has_value(), selIdx);
```

W [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp), w pętli rysowania (~993):
```cpp
  int propIdx = 0;
  for (const PlacedProp& p : scene.props()) {
    // ... istniejący kod wyboru modelu ...

    float spec = 0.1f;
    if (p.kind == PropKind::Car) {
      spec = 0.16f;
    }
    
    // Podświetlenie zaznaczonego auta
    glm::vec3 tint(1.0f);
    if (propIdx == selectedPropIndex && p.kind == PropKind::Car) {
      tint = glm::vec3(0.3f, 1.0f, 0.3f);  // zielony tint
      spec = 0.6f;  // mocniejszy połysk
    }

    glm::mat4 m = glm::translate(glm::mat4(1.0f), p.position);
    m = glm::rotate(m, p.rotY, glm::vec3(0.0f, 1.0f, 0.0f));
    m = glm::scale(m, glm::vec3(p.scale));
    model->draw(m, vp, lightVP, modelShader_, lightDir, camPos, amb, spec, kShadowUnit, kModelDiffuseUnit, whiteTex_);
    ++propIdx;
  }
```

> [!NOTE]
> Żeby tint faktycznie zadziałał, musiałbyś przekazać go jako uniform do shadera `model.frag` i pomnożyć `albedo *= uTint`. Uproszczona wersja: zmień specular na mocny aby auto „błyszczało" gdy jest zaznaczone — to prostsze i nie wymaga zmian w shaderze.

---

### Zadanie 13: „Dodaj możliwość zmiany modelu samochodu klawiszem M (cyklicznie: AE86 → GR86 → Avalon → AE86...)"

**Pliki**: [Application.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.hpp), [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp), [ParkingScene.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingScene.hpp), [Input.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/input/Input.cpp)

**Krok 1** — W [ParkingScene.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingScene.hpp), dodaj:
```cpp
  /// Mutowalny dostęp do propsów (np. zmiana modelu wybranego auta).
  std::vector<PlacedProp>& mutableProps() { return props_; }
```

**Krok 2** — W [Application.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.hpp):
```cpp
public:
  void cycleSelectedCarModel();
```

**Krok 3** — W [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp):
```cpp
void Application::cycleSelectedCarModel() {
  if (!selectedCarPropIndex_.has_value()) return;
  size_t i = *selectedCarPropIndex_;
  auto& props = scene_.mutableProps();
  if (i >= props.size() || props[i].kind != PropKind::Car) return;
  props[i].carModel = static_cast<uint8_t>((props[i].carModel + 1) % 3);
}
```

**Krok 4** — W [Input.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/input/Input.cpp):
```cpp
    case GLFW_KEY_M:
      if (h->app) { h->app->cycleSelectedCarModel(); }
      break;
```

---

### Zadanie 14: „Dodaj obrót zaznaczonego auta o 180° klawiszem V (auto stoi tyłem / przodem)"

**Pliki**: [Application.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.hpp), [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp), [Input.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/input/Input.cpp)

**Krok 1** — W [Application.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.hpp):
```cpp
public:
  void flipSelectedCar();
```

**Krok 2** — W [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp):
```cpp
void Application::flipSelectedCar() {
  if (!selectedCarPropIndex_.has_value()) return;
  size_t i = *selectedCarPropIndex_;
  auto& props = scene_.mutableProps();
  if (i >= props.size() || props[i].kind != PropKind::Car) return;
  constexpr float kPi = 3.14159265f;
  props[i].rotY += kPi;
  // Znormalizuj do [0, 2π]
  if (props[i].rotY > 2.0f * kPi) props[i].rotY -= 2.0f * kPi;
}
```

**Krok 3** — W [Input.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/input/Input.cpp):
```cpp
    case GLFW_KEY_V:
      if (h->app) { h->app->flipSelectedCar(); }
      break;
```

---

## 💡 OŚWIETLENIE I EFEKTY

---

### Zadanie 15: „Dodaj płynne przejście dzień↔noc (zamiast natychmiastowego)"

**Pliki**: [Lighting.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/lighting/Lighting.hpp), [Lighting.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/lighting/Lighting.cpp)

**Krok 1** — W [Lighting.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/lighting/Lighting.hpp):
```diff
+ private:
+   float transition_{0.0f};  // 0 = noc, 1 = dzień
+ public:
+   void update(float dt);
```

Zmień `setMode`:
```diff
- void setMode(LightingMode mode) { mode_ = mode; }
+ void setMode(LightingMode mode) { mode_ = mode; }
```

**Krok 2** — W [Lighting.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/lighting/Lighting.cpp):
```cpp
void Lighting::update(float dt) {
  const float target = (mode_ == LightingMode::Day) ? 1.0f : 0.0f;
  const float speed = 1.2f;  // pełne przejście w ~0.8s
  if (transition_ < target) {
    transition_ = std::min(transition_ + speed * dt, target);
  } else if (transition_ > target) {
    transition_ = std::max(transition_ - speed * dt, target);
  }
}

glm::vec3 Lighting::clearColor() const {
  glm::vec3 night(0.0f, 0.0f, 0.0f);
  glm::vec3 day(0.55f, 0.68f, 0.88f);
  return glm::mix(night, day, transition_);
}

float Lighting::ambientFactor() const {
  return 0.42f * transition_;
}

float Lighting::directionalLightWeight() const {
  return transition_;
}
```

Dodaj include:
```cpp
#include <algorithm>
#include <cmath>
```

**Krok 3** — Wywołaj `update()` w pętli głównej. W [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp) `run()`, przed `renderer_.draw(...)`:
```cpp
    scene_.lighting().update(dt);
```

---

### Zadanie 16: „Dodaj migotanie lamp w nocy (losowe krótkie zaciemnienia)"

**Plik**: [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp), ~linia 805

Zmień obliczanie `pointIntensity`:
```diff
- const float pointIntensity =
-     (scene.lighting().mode() == LightingMode::Night ? 10.5f : 3.2f) * kLampPointIntensityScale *
-     (scene.lighting().mode() == LightingMode::Night ? kNightLampExtraScale : 1.0f);
+ float basePointIntensity =
+     (scene.lighting().mode() == LightingMode::Night ? 10.5f : 3.2f) * kLampPointIntensityScale *
+     (scene.lighting().mode() == LightingMode::Night ? kNightLampExtraScale : 1.0f);
+ // Migotanie lamp w nocy
+ if (scene.lighting().mode() == LightingMode::Night) {
+   float flicker = 0.92f + 0.08f * std::sin(timeSec * 13.7f) * std::sin(timeSec * 7.3f);
+   basePointIntensity *= flicker;
+ }
+ const float pointIntensity = basePointIntensity;
```

**Wyjaśnienie**: Mnożymy intensywność przez sinusoidę — daje efekt subtelnego „migotania" lamp. Dwie sinusoidy o różnych częstotliwościach (13.7 i 7.3 Hz) tworzą nieregularny wzorzec.

---

### Zadanie 17: „Zrób, żeby intensywność lamp (jasność światła nocnego) była konfigurowalna klawiszami 1/2"

**Pliki**: [Lighting.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/lighting/Lighting.hpp), [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp)

**Krok 1** — W [Lighting.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/lighting/Lighting.hpp):
```cpp
  float lampBrightness_{1.0f};
  void setLampBrightness(float b) { lampBrightness_ = std::clamp(b, 0.1f, 3.0f); }
  float lampBrightness() const { return lampBrightness_; }
```

**Krok 2** — W [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp) ~linia 805, pomnóż:
```diff
  const float pointIntensity =
      (scene.lighting().mode() == LightingMode::Night ? 10.5f : 3.2f) * kLampPointIntensityScale *
-     (scene.lighting().mode() == LightingMode::Night ? kNightLampExtraScale : 1.0f);
+     (scene.lighting().mode() == LightingMode::Night ? kNightLampExtraScale : 1.0f) *
+     scene.lighting().lampBrightness();
```

**Krok 3** — W [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp) `handleParkingSettingsKeys()`:
```cpp
  const bool key1 = glfwGetKey(w, GLFW_KEY_1) == GLFW_PRESS;
  const bool key2 = glfwGetKey(w, GLFW_KEY_2) == GLFW_PRESS;
  if (key1) {
    scene_.lighting().setLampBrightness(scene_.lighting().lampBrightness() - 0.02f);
    updateWindowTitle();
  }
  if (key2) {
    scene_.lighting().setLampBrightness(scene_.lighting().lampBrightness() + 0.02f);
    updateWindowTitle();
  }
```

---

## 🏗️ PARKING — logika generowania

---

### Zadanie 18: „Dodaj kolorowe strefy parkingowe — np. co drugi rząd inny kolor asfaltu"

**Plik**: [ground.frag](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/assets/shaders/ground.frag)

W bloku `main()`, zmień sekcję parking (~77-78):
```diff
  if (abs(x) <= uHalfParkingL && abs(z) <= uHalfParkingW) {
-   base = colPark;
+   // Co 22 metry (spotDepth + aisle = ~22m) zmień odcień
+   float stripe = mod(z + uHalfParkingW, 44.5) / 44.5;
+   vec3 zone1 = colPark * vec3(1.0, 0.95, 0.9);   // ciepłoszary
+   vec3 zone2 = colPark * vec3(0.9, 0.95, 1.0);   // chłodnoszary
+   base = mix(zone1, zone2, step(0.5, stripe));
  }
```

**Wyjaśnienie**: Modyfikacja **w shaderze** — nie trzeba rekompilować C++! Zmień plik `.frag`, zapisz i uruchom program ponownie. Shader użyje pozycji Z do naprzemiennego kolorowania rzędów.

---

### Zadanie 19: „Dodaj oznaczenia miejsc dla niepełnosprawnych (np. pierwsze 2 miejsca w każdym rzędzie mają niebieskie linie)"

**Plik**: [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp)

Po narysowaniu normalnych linii (~948), dodaj dodatkowe niebieskie linie:

```cpp
  // Niebieskie linie — miejsca dla niepełnosprawnych (pierwsze 2 w każdym rzędzie)
  {
    static std::vector<float> handicapLines;
    handicapLines.clear();
    const int rowCount = std::max(2, gen.rowCount());
    const float spotDepth = ParkingGenerator::spotDepthMeters();
    const float aisle = ParkingGenerator::aisleWidthMeters();
    const float halfL = gen.length() * 0.5f;
    const float halfW = gen.width() * 0.5f;
    const float dxSpot = gen.length() / static_cast<float>(std::max(1, gen.spotsPerRow()));
    const float step = spotDepth + aisle;
    const float startZ = -halfW + spotDepth * 0.5f;
    const float yLine = 0.10f;

    constexpr int kHandicapSpots = 2;
    for (int k = 0; k < rowCount; ++k) {
      const float zc = startZ + static_cast<float>(k) * step;
      const float zMin = zc - spotDepth * 0.5f;
      const float zMax = zc + spotDepth * 0.5f;
      for (int i = 0; i < std::min(kHandicapSpots, gen.spotsPerRow()); ++i) {
        const float x0 = -halfL + static_cast<float>(i) * dxSpot;
        const float x1 = x0 + dxSpot;
        // Krzyżyk wewnątrz miejsca
        appendLine(handicapLines, yLine, x0, zMin, x1, zMax);
        appendLine(handicapLines, yLine, x0, zMax, x1, zMin);
      }
    }

    if (!handicapLines.empty()) {
      glBindBuffer(GL_ARRAY_BUFFER, lineVbo_);
      glBufferData(GL_ARRAY_BUFFER,
                   static_cast<GLsizeiptr>(handicapLines.size() * sizeof(float)),
                   handicapLines.data(), GL_DYNAMIC_DRAW);
      flatShader_.use();
      flatShader_.setMat4("uMVP", glm::value_ptr(vp));
      flatShader_.setVec3("uColor", 0.2f, 0.4f, 0.95f);  // niebieski
      flatShader_.setFloat("uAmbient", std::max(amb, 0.55f));
      flatShader_.setFloat("uAlpha", 1.0f);
      glLineWidth(2.5f);
      glBindVertexArray(lineVao_);
      glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(handicapLines.size() / 3));
      glBindVertexArray(0);
      glLineWidth(1.5f);
    }
  }
```

---

### Zadanie 20: „Dodaj losowe kolory samochodów (tint per auto)"

> [!NOTE]
> To wymaga zmiany w shaderze. Dodajemy uniform `uColorTint` do `model.frag`.

**Krok 1** — W [model.frag](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/assets/shaders/model.frag), dodaj uniform:
```diff
  uniform float uSpecularStrength;
+ uniform vec3 uColorTint;
```

I użyj go:
```diff
- vec3 albedo = uUseTexture ? texture(uDiffuse, vTex).rgb * uBaseColor : uBaseColor;
+ vec3 albedo = uUseTexture ? texture(uDiffuse, vTex).rgb * uBaseColor * uColorTint : uBaseColor * uColorTint;
```

**Krok 2** — W [PlacedProp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingScene.hpp), dodaj pole:
```diff
  struct PlacedProp {
    PropKind kind{PropKind::Lamp};
    glm::vec3 position{};
    float rotY{0.0f};
    float scale{1.0f};
    uint8_t carModel{0};
+   glm::vec3 colorTint{1.0f, 1.0f, 1.0f};
  };
```

**Krok 3** — W [ParkingScene.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingScene.cpp) `rebuildPlacements()`, przy tworzeniu aut (~234), ustaw losowy kolor:
```diff
+ std::uniform_real_distribution<float> colorDist(0.6f, 1.0f);
+ glm::vec3 tint(colorDist(rng), colorDist(rng), colorDist(rng));
- props_.push_back({PropKind::Car, {s.x, kCarY, s.z}, s.yaw, kCarScale, modelIdx});
+ props_.push_back({PropKind::Car, {s.x, kCarY, s.z}, s.yaw, kCarScale, modelIdx, tint});
```

**Krok 4** — W [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp), w pętli rysowania (~993), przed `model->draw(...)`:
```cpp
    if (p.kind == PropKind::Car) {
      modelShader_.setVec3("uColorTint", p.colorTint.x, p.colorTint.y, p.colorTint.z);
    } else {
      modelShader_.setVec3("uColorTint", 1.0f, 1.0f, 1.0f);
    }
```

---

### Zadanie 21: „Dodaj wyświetlanie FPS (klatek na sekundę) w lewym dolnym rogu ekranu"

**Pliki**: [Application.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.hpp), [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp)

**Krok 1** — W [Application.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.hpp):
```cpp
  int fpsCounter_{0};
  float fpsTimer_{0.0f};
  int lastFps_{0};
```

**Krok 2** — W [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp) `run()`, po obliczeniu `dt`:
```cpp
    fpsCounter_++;
    fpsTimer_ += dt;
    if (fpsTimer_ >= 1.0f) {
      lastFps_ = fpsCounter_;
      fpsCounter_ = 0;
      fpsTimer_ -= 1.0f;
    }
```

**Krok 3** — Przekaż FPS do renderera. Najprostszy sposób — wyświetl w tytule okna. W `updateWindowTitle()`:
```diff
    std::snprintf(buf, sizeof(buf),
-                 "Parking — Auta: ...",
+                 "Parking — FPS: %d | OpenGL 3.3", lastFps_);
```

Lub dodaj to do `drawOverlayUi()` w [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp) (bardziej zaawansowane — jako HUD):

Zmień sygnaturę `draw()` żeby przyjmowała FPS, lub prostszym sposobem — wyświetl to w panelu ustawień w `drawOverlayUi()` dodając nową linię tekstu.

---

### Zadanie 22: „Dodaj klawisz H który ukrywa/pokazuje panel HUD (ikonka w rogu)"

**Pliki**: [Application.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.hpp), [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp), [Input.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/input/Input.cpp)

**Krok 1** — W [Application.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.hpp):
```cpp
  bool hudVisible_{true};
public:
  void toggleHud() { hudVisible_ = !hudVisible_; }
  bool hudVisible() const { return hudVisible_; }
```

**Krok 2** — W [Input.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/input/Input.cpp):
```cpp
    case GLFW_KEY_H:
      if (h->app) { h->app->toggleHud(); }
      break;
```

**Krok 3** — W [Renderer.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.hpp), zmień sygnaturę:
```diff
  void draw(ParkingScene& scene, Camera& camera, float timeSec, bool parkingSettingsOpen,
-           bool carAwaitingDestination);
+           bool carAwaitingDestination, bool hudVisible = true);
```

**Krok 4** — W [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp), w `draw()` (~1021):
```diff
- drawOverlayUi(parkingSettingsOpen, gen, scene.lighting().mode(), carAwaitingDestination);
+ if (hudVisible) {
+   drawOverlayUi(parkingSettingsOpen, gen, scene.lighting().mode(), carAwaitingDestination);
+ }
```

**Krok 5** — W [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp) `run()`:
```diff
- renderer_.draw(scene_, camera_, static_cast<float>(now), settingsOpen_, selectedCarPropIndex_.has_value());
+ renderer_.draw(scene_, camera_, static_cast<float>(now), settingsOpen_, selectedCarPropIndex_.has_value(), hudVisible_);
```

---

### Zadanie 23: „Dodaj klawisz Q do zmiany grubości linii parkingowych"

**Pliki**: [Application.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.hpp), [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp), [Renderer.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.hpp), [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp)

**Krok 1** — W [Renderer.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.hpp):
```cpp
  float parkingLineWidth_{1.5f};
public:
  void setLineWidth(float w) { parkingLineWidth_ = std::clamp(w, 1.0f, 5.0f); }
  float lineWidth() const { return parkingLineWidth_; }
```

**Krok 2** — W [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp), linia ~855:
```diff
- glLineWidth(1.5f);
+ glLineWidth(parkingLineWidth_);
```

**Krok 3** — W [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp) `handleParkingSettingsKeys()`:
```cpp
  const bool keyQ = glfwGetKey(w, GLFW_KEY_Q) == GLFW_PRESS;
  if (keyQ) {
    float lw = renderer_.lineWidth() + 0.05f;
    if (lw > 5.0f) lw = 1.0f;  // cyklicznie
    renderer_.setLineWidth(lw);
  }
```

---

### Zadanie 24: „Dodaj eksport statystyk do konsoli klawiszem I"

**Plik**: [Input.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/input/Input.cpp):

```cpp
    case GLFW_KEY_I: {
      if (!h->scene) break;
      const auto& gen = h->scene->generator();
      int cars = 0, lamps = 0;
      for (const auto& p : h->scene->props()) {
        if (p.kind == parking::PropKind::Car) ++cars;
        else if (p.kind == parking::PropKind::Lamp) ++lamps;
      }
      std::printf("\n=== STATYSTYKI PARKINGU ===\n");
      std::printf("Miejsca na rzad: %d\n", gen.spotsPerRow());
      std::printf("Liczba rzedow:   %d\n", gen.rowCount());
      std::printf("Razem miejsc:    %d\n", gen.spotCount());
      std::printf("Dlugosc:         %.1f m\n", static_cast<double>(gen.length()));
      std::printf("Szerokosc:       %.1f m\n", static_cast<double>(gen.width()));
      std::printf("Zaparkowane:     %d\n", cars);
      std::printf("Wolne:           %d\n", gen.spotCount() - cars);
      std::printf("Lampy:           %d\n", lamps);
      std::printf("===========================\n");
      break;
    }
```

Dodaj include:
```cpp
#include "scene/ParkingScene.hpp"
#include <cstdio>
```

---

### Zadanie 25: „Dodaj klawisz L do przełączania widoczności linii parkingowych (pokaż/ukryj)"

**Pliki**: [Application.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.hpp), [Renderer.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.hpp), [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp), [Input.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/input/Input.cpp)

**Krok 1** — W [Renderer.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.hpp):
```cpp
  bool showParkingLines_{true};
public:
  void toggleLines() { showParkingLines_ = !showParkingLines_; }
```

**Krok 2** — W [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp), owinąć rysowanie linii (~933-968) w warunek:
```diff
+ if (showParkingLines_) {
  if (!lineVerts.empty()) {
    // ... istniejący kod rysowania linii ...
  }
  if (!centerLineVerts.empty()) {
    // ... istniejący kod rysowania środkowej linii ...
  }
+ }
```

**Krok 3** — W [Input.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/input/Input.cpp):
```cpp
    case GLFW_KEY_L:
      if (h->renderer) { h->renderer->toggleLines(); }
      break;
```

---

### Zadanie 26: „Dodaj wyświetlanie numeru miejsca parkingowego nad każdym miejscem (jako HUD 3D)"

> [!NOTE]
> To jest zaawansowane — wymaga rysowania tekstu w przestrzeni 3D. Uproszczona wersja: narysuj małe krzyżyki/kółka na środku wolnych miejsc, żeby było widać które są wolne.

**Plik**: [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp)

Dodaj w `draw()`, po rysowaniu linii parkingowych (~968):

```cpp
  // Markery wolnych miejsc (zielone punkty na środku wolnych miejsc)
  {
    static std::vector<float> markerVerts;
    markerVerts.clear();
    const float halfL = gen.length() * 0.5f;
    const float halfW = gen.width() * 0.5f;
    const float spotDepth = ParkingGenerator::spotDepthMeters();
    const float aisleW = ParkingGenerator::aisleWidthMeters();
    const int rowCount = std::max(2, gen.rowCount());
    const int spotsPerRow = std::max(1, gen.spotsPerRow());
    const float dx = gen.length() / static_cast<float>(spotsPerRow);
    const float step = spotDepth + aisleW;
    const float startZ = -halfW + spotDepth * 0.5f;

    for (int k = 0; k < rowCount; ++k) {
      const float z = startZ + static_cast<float>(k) * step;
      for (int i = 0; i < spotsPerRow; ++i) {
        const float x = -halfL + (static_cast<float>(i) + 0.5f) * dx;
        // Sprawdź czy wolne
        bool occupied = false;
        for (const auto& p : scene.props()) {
          if (p.kind != PropKind::Car) continue;
          float ddx = p.position.x - x;
          float ddz = p.position.z - z;
          if (ddx * ddx + ddz * ddz < (dx * 0.35f) * (dx * 0.35f)) {
            occupied = true; break;
          }
        }
        if (!occupied) {
          float sz = 0.5f;
          appendLine(markerVerts, 0.11f, x - sz, z, x + sz, z);
          appendLine(markerVerts, 0.11f, x, z - sz, x, z + sz);
        }
      }
    }

    if (!markerVerts.empty()) {
      glBindBuffer(GL_ARRAY_BUFFER, lineVbo_);
      glBufferData(GL_ARRAY_BUFFER,
                   static_cast<GLsizeiptr>(markerVerts.size() * sizeof(float)),
                   markerVerts.data(), GL_DYNAMIC_DRAW);
      flatShader_.use();
      flatShader_.setMat4("uMVP", glm::value_ptr(vp));
      flatShader_.setVec3("uColor", 0.1f, 0.85f, 0.2f);  // zielony
      flatShader_.setFloat("uAmbient", std::max(amb, 0.5f));
      flatShader_.setFloat("uAlpha", 0.8f);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glLineWidth(2.0f);
      glBindVertexArray(lineVao_);
      glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(markerVerts.size() / 3));
      glBindVertexArray(0);
      glDisable(GL_BLEND);
      glLineWidth(1.5f);
    }
  }
```

---

### Zadanie 27: „Zrób, żeby kliknięcie na wolne miejsce (bez zaznaczonego auta) dodawało nowe auto"

**Plik**: [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp) — metoda `handleWorldLeftClick()` (~103-127)

Zmień końcówkę metody:
```diff
  if (const auto picked = scene_.pickCarPropIndex(rayOrigin, rayDir)) {
    selectedCarPropIndex_ = *picked;
- }
+ } else if (groundHit) {
+   // Klik na wolne miejsce → dodaj auto
+   // Użyj tryMoveCarToWorldXZ z tymczasowym autem
+   // Prostsze: dodaj auto na pozycji kliknięcia, jeśli to miejsce parkingowe
+   auto& props = scene_.mutableProps();
+   PlacedProp newCar{};
+   newCar.kind = PropKind::Car;
+   newCar.position = glm::vec3(groundPoint.x, 0.06f, groundPoint.z);
+   newCar.scale = 1.0f;
+   newCar.carModel = static_cast<uint8_t>(props.size() % 3);
+   props.push_back(newCar);
+   // Spróbuj snap do najbliższego miejsca (re-use logiki tryMoveCarToWorldXZ)
+   size_t idx = props.size() - 1;
+   if (!scene_.tryMoveCarToWorldXZ(idx, groundPoint.x, groundPoint.z)) {
+     // Nie udało się znaleźć legalnego miejsca — usuń
+     props.pop_back();
+   }
+ }
```

Musisz dodać metodę `mutableProps()` do `ParkingScene` (patrz Zadanie 13 Krok 1).

---

### Zadanie 28: „Dodaj pasek postępu zajętości parkingu na górze ekranu"

**Plik**: [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp) — w `drawOverlayUi()`, przed `drawHudCornerOverlay(...)`:

```cpp
  // Pasek zajętości parkingu
  {
    int carCount = 0;
    for (const auto& p : scene.props()) {  // potrzebujesz dostępu do scene — dodaj parametr
      if (p.kind == PropKind::Car) ++carCount;
    }
    // ... ale drawOverlayUi nie ma dostępu do scene. Alternatywa: przekaż gen.spotCount() i carCount
  }
```

Prostsze rozwiązanie — dodaj parametry do `drawOverlayUi`. Lub narysuj pasek na podstawie danych z `gen`:

```cpp
  // Pasek zajętości — dodaj do drawOverlayUi (potrzebujesz dodać parametr int carCount i int totalSpots)
  {
    constexpr float barW = 300.0f;
    constexpr float barH = 12.0f;
    const float bx0 = (fw - barW) * 0.5f;
    const float by0 = fh - 8.0f;
    float fillRatio = static_cast<float>(carCount) / static_cast<float>(std::max(1, totalSpots));
    fillRatio = std::clamp(fillRatio, 0.0f, 1.0f);

    // Tło paska
    const float bgVerts[] = {
        bx0, by0, 0.f, bx0 + barW, by0, 0.f, bx0 + barW, by0 + barH, 0.f,
        bx0, by0, 0.f, bx0 + barW, by0 + barH, 0.f, bx0, by0 + barH, 0.f,
    };
    glBindBuffer(GL_ARRAY_BUFFER, lineVbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bgVerts), bgVerts, GL_DYNAMIC_DRAW);
    flatShader_.setMat4("uMVP", glm::value_ptr(ortho));
    flatShader_.setVec3("uColor", 0.15f, 0.15f, 0.18f);
    flatShader_.setFloat("uAmbient", 1.0f);
    flatShader_.setFloat("uAlpha", 0.7f);
    glBindVertexArray(lineVao_);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Wypełnienie
    const float fillW = barW * fillRatio;
    const float fillVerts[] = {
        bx0, by0, 0.f, bx0 + fillW, by0, 0.f, bx0 + fillW, by0 + barH, 0.f,
        bx0, by0, 0.f, bx0 + fillW, by0 + barH, 0.f, bx0, by0 + barH, 0.f,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(fillVerts), fillVerts, GL_DYNAMIC_DRAW);
    // Kolor: zielony→żółty→czerwony w zależności od zajętości
    float r = std::min(1.0f, fillRatio * 2.0f);
    float g = std::min(1.0f, (1.0f - fillRatio) * 2.0f);
    flatShader_.setVec3("uColor", r, g, 0.1f);
    flatShader_.setFloat("uAlpha", 0.85f);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
  }
```

---

> [!TIP]
> **Szybka ściąga — najczęstsze wzorce:**
>
> | Chcesz... | Wzorzec |
> |-----------|---------|
> | Dodać klawisz (jednorazowy) | `Input.cpp` → `case GLFW_KEY_X:` w `switch` |
> | Dodać klawisz (trzymany) | `Application.cpp` → `glfwGetKey(w, GLFW_KEY_X) == GLFW_PRESS` w `updateCamera` lub `handleParkingSettingsKeys` |
> | Dodać flagę toggle | Pole `bool` w `Application.hpp` + metoda `toggle()` + klawisz w `Input.cpp` |
> | Zmienić kamerę | `camera_.orbit(yaw, pitch)` / `camera_.zoom(delta)` / `camera_.setOrbit(dist, yaw, pitch, target)` |
> | Dodać/usunąć auto | `scene_.mutableProps().push_back(...)` / `scene_.removeCarProp(idx)` |
> | Narysować linie | `appendLine(verts, y, x0, z0, x1, z1)` → `glDrawArrays(GL_LINES, ...)` |
> | Narysować tekst HUD | `appendEasyFontTriangles(...)` → `flushHudTriangles(...)` |
> | Zmienić kolor | `shader.setVec3("uColor", R, G, B)` (wartości 0.0–1.0) |
