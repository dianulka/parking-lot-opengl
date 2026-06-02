# 🎓 Przykładowe zadania od prowadzącego — pełne rozwiązania

> [!IMPORTANT]
> Każde zadanie zawiera: **pytanie**, **gdzie edytować**, **dokładny kod before/after**, **wyjaśnienie**. 
> Zadania pogrupowane od najłatwiejszych do najtrudniejszych.

---

## 📗 POZIOM ŁATWY — zmiana jednej wartości / linii

---

### Zadanie 1: „Zmień kolor linii parkingowych na żółty"

**Plik**: [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp) linia ~940

```diff
- flatShader_.setVec3("uColor", 0.92f, 0.92f, 0.85f);
+ flatShader_.setVec3("uColor", 1.0f, 0.9f, 0.0f);
```

**Wyjaśnienie**: `setVec3("uColor", R, G, B)` ustawia kolor w zakresie 0.0–1.0. (1.0, 0.9, 0.0) = żółty.

---

### Zadanie 2: „Zmień kolor środkowej linii drogi na czerwony"

**Plik**: [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp) linia ~959

```diff
- flatShader_.setVec3("uColor", 0.92f, 0.82f, 0.18f);
+ flatShader_.setVec3("uColor", 1.0f, 0.0f, 0.0f);
```

---

### Zadanie 3: „Zmień domyślną liczbę miejsc na rząd na 8"

**Plik**: [ParkingGenerator.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingGenerator.hpp) linia ~35

```diff
- static constexpr int kDefaultSpotsPerRow = 16;
+ static constexpr int kDefaultSpotsPerRow = 8;
```

**Wyjaśnienie**: To zmienia wartość startową przy uruchomieniu bez argumentów CLI.

---

### Zadanie 4: „Zmień domyślną liczbę rzędów na 4"

**Plik**: [ParkingGenerator.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingGenerator.hpp) linia ~28

```diff
- static constexpr int kDefaultRows = 2;
+ static constexpr int kDefaultRows = 4;
```

---

### Zadanie 5: „Zmień kolor nieba w trybie dziennym na zachód słońca (pomarańczowy)"

**Plik**: [Lighting.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/lighting/Lighting.cpp) linia ~9

```diff
- return glm::vec3(0.55f, 0.68f, 0.88f);
+ return glm::vec3(0.92f, 0.55f, 0.30f);
```

**Wyjaśnienie**: `clearColor()` zwraca kolor tła — to główny gradient nieba. RGB (0.92, 0.55, 0.30) = ciepły pomarańcz.

---

### Zadanie 6: „Zmień kolor nieba nocnego na granatowy zamiast czarnego"

**Plik**: [Lighting.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/lighting/Lighting.cpp) linia ~7

```diff
- return glm::vec3(0.0f, 0.0f, 0.0f);
+ return glm::vec3(0.02f, 0.02f, 0.08f);
```

---

### Zadanie 7: „Zwiększ jasność sceny dziennej (mniej ciemnych cieni)"

**Plik**: [Lighting.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/lighting/Lighting.cpp) linia ~16

```diff
- return 0.42f;
+ return 0.65f;
```

**Wyjaśnienie**: `ambientFactor()` określa minimalną jasność w cieniu. 0.42 = umiarkowane cienie, 0.65 = jaśniej.

---

### Zadanie 8: „Zmień rozdzielczość okna na 1920×1080"

**Plik**: [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp) linia ~30

```diff
- window_(1280, 720, kTitleNormal),
+ window_(1920, 1080, kTitleNormal),
```

---

### Zadanie 9: „Zmień tytuł okna"

**Plik**: [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp) linia ~25

```diff
- constexpr char kTitleNormal[] = "Parking prostokątny — OpenGL 3.3";
+ constexpr char kTitleNormal[] = "Symulacja parkingu 3D — Jan Kowalski";
```

---

### Zadanie 10: „Zmień kolor nawierzchni parkingu na ciemniejszy"

**Plik**: [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp) linia ~874

```diff
- groundShader_.setVec3("uParkingTint", 0.62f, 0.62f, 0.64f);
+ groundShader_.setVec3("uParkingTint", 0.35f, 0.35f, 0.38f);
```

---

### Zadanie 11: „Zmień kolor trawy na jaśniejszy zielony"

**Plik**: [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp) linia ~872

```diff
- groundShader_.setVec3("uGrassTint", 0.55f, 0.95f, 0.48f);
+ groundShader_.setVec3("uGrassTint", 0.45f, 1.0f, 0.35f);
```

---

### Zadanie 12: „Zmień kolor drogi dojazdowej"

**Plik**: [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp) linia ~873

```diff
- groundShader_.setVec3("uRoadStripTint", 0.58f, 0.58f, 0.6f);
+ groundShader_.setVec3("uRoadStripTint", 0.45f, 0.45f, 0.50f);
```

---

### Zadanie 13: „Zwiększ maksymalną liczbę rzędów z 6 do 8"

**Plik**: [ParkingGenerator.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingGenerator.hpp) linia ~26

```diff
- static constexpr int kMaxRows = 6;
+ static constexpr int kMaxRows = 8;
```

---

### Zadanie 14: „Zmień prędkość przesuwania kamery (WASD)"

**Plik**: [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp) linia ~22

```diff
- constexpr float kCameraPanSpeed = 38.0f;
+ constexpr float kCameraPanSpeed = 60.0f;
```

---

### Zadanie 15: „Zmień domyślny tryb na dzień (zamiast nocy)"

**Plik**: [Lighting.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/lighting/Lighting.hpp) linia ~26

```diff
- LightingMode mode_{LightingMode::Night};
+ LightingMode mode_{LightingMode::Day};
```

---

## 📙 POZIOM ŚREDNI — zmiana kilku linii / dodanie prostej logiki

---

### Zadanie 16: „Niech program startuje z 100% zajętymi miejscami"

**Plik**: [ParkingScene.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingScene.cpp) linia ~201-204

```diff
- int targetCars = (totalSpots * 2) / 5;
- targetCars = std::max(1, targetCars);
- const int maxCars = std::min(20, std::max(1, totalSpots - 1));
- targetCars = std::min(targetCars, maxCars);
+ int targetCars = totalSpots;  // 100% zajętych
+ const int maxCars = totalSpots;
```

**Wyjaśnienie**: Usuwamy limit 40% i max 20 aut — teraz wszystkie miejsca będą zajęte.

---

### Zadanie 17: „Dodaj klawisz R do resetowania kamery do widoku domyślnego"

**Plik**: [Input.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/input/Input.cpp) linia ~29-41

```diff
  switch (key) {
    case GLFW_KEY_ESCAPE:
      if (h->app && h->app->consumeEscapeForSettings()) {
        break;
      }
      if (h->app && h->app->consumeEscapeForCarSelection()) {
        break;
      }
      glfwSetWindowShouldClose(window, GLFW_TRUE);
      break;
+   case GLFW_KEY_R:
+     if (h->camera) {
+       h->camera->setOrbit(52.0f, 39.0f, 61.5f, glm::vec3(0.0f, 0.0f, 0.0f));
+     }
+     break;
    default:
      break;
  }
```

Dodaj też include na górze pliku Input.cpp:
```diff
+ #include "rendering/Camera.hpp"
```

**Wyjaśnienie**: `setOrbit()` ustawia kamerę w domyślnej pozycji (distance=52, yaw=39°, pitch=61.5°, target=origin).

---

### Zadanie 18: „Zmień kierunek światła słonecznego (np. słońce z zachodu)"

**Plik**: [Lighting.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/lighting/Lighting.cpp) linia ~31

```diff
- return glm::normalize(glm::vec3(0.35f, 0.85f, 0.45f));
+ return glm::normalize(glm::vec3(-0.8f, 0.6f, 0.1f));
```

**Wyjaśnienie**: Wektor `(x, y, z)` wskazuje **w stronę** słońca. `y` = wysokość nad horyzontem. Ujemny `x` = słońce z lewej strony.

---

### Zadanie 19: „Zwiększ rozmiar samochodów o 20%"

**Plik**: [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp) linia ~457

```diff
- constexpr float kCarNormalizeM = 4.5f;
+ constexpr float kCarNormalizeM = 5.4f;
```

**Wyjaśnienie**: `kCarNormalizeM` = długość auta w metrach. 4.5 × 1.2 = 5.4.

---

### Zadanie 20: „Zmień kolor światła lamp nocnych na niebieski"

**Plik**: [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp) — trzeba zmienić w **4 miejscach** (uniformy `uPointColor`):

Linia ~891:
```diff
- groundShader_.setVec3("uPointColor", 1.0f, 0.93f, 0.72f);
+ groundShader_.setVec3("uPointColor", 0.5f, 0.7f, 1.0f);
```

Linia ~921 (w `grassBlades_.draw(...)` — parametr `glm::vec3(1.0f, 0.93f, 0.72f)`):
```diff
- glm::vec3(1.0f, 0.93f, 0.72f));
+ glm::vec3(0.5f, 0.7f, 1.0f));
```

Linia ~982:
```diff
- modelShader_.setVec3("uPointColor", 1.0f, 0.93f, 0.72f);
+ modelShader_.setVec3("uPointColor", 0.5f, 0.7f, 1.0f);
```

---

### Zadanie 21: „Zmień FOV kamery (pole widzenia)"

**Plik**: [Camera.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Camera.hpp) linia ~38

```diff
- float fovDegrees_{52.0f};
+ float fovDegrees_{70.0f};
```

**Wyjaśnienie**: Mniejszy FOV = bardziej „teleobiektyw" (flat). Większy = szerokie pole widzenia, więcej perspektywy.

---

### Zadanie 22: „Zmień minimalny / maksymalny zoom kamery"

**Plik**: [Camera.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Camera.cpp) linia ~49

```diff
- distance_ = std::clamp(distance_, 8.0f, 420.0f);
+ distance_ = std::clamp(distance_, 5.0f, 600.0f);
```

**Wyjaśnienie**: Pierwszy parametr = minimum (najbliżej), drugi = max (najdalej).

---

### Zadanie 23: „Zmień minimalny kąt kamery (pozwól patrzeć bardziej z boku)"

**Plik**: [Camera.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Camera.cpp) linia ~43

```diff
- pitchDeg_ = std::clamp(pitchDeg_, 18.0f, 88.0f);
+ pitchDeg_ = std::clamp(pitchDeg_, 5.0f, 88.0f);
```

**Wyjaśnienie**: Pitch 18° = nie da się patrzeć zbyt z boku. 5° = prawie na poziomie gruntu.

---

### Zadanie 24: „Dodaj klawisz F do przełączania pełnego ekranu"

**Plik**: [Input.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/input/Input.cpp)

```diff
  switch (key) {
    case GLFW_KEY_ESCAPE:
      // ...
      break;
+   case GLFW_KEY_F: {
+     GLFWmonitor* monitor = glfwGetWindowMonitor(window);
+     if (monitor) {
+       // Jest fullscreen → wróć do okna
+       glfwSetWindowMonitor(window, nullptr, 100, 100, 1280, 720, 0);
+     } else {
+       // Jest okno → przejdź na fullscreen
+       GLFWmonitor* primary = glfwGetPrimaryMonitor();
+       const GLFWvidmode* mode = glfwGetVideoMode(primary);
+       glfwSetWindowMonitor(window, primary, 0, 0, mode->width, mode->height, mode->refreshRate);
+     }
+     break;
+   }
    default:
      break;
  }
```

---

### Zadanie 25: „Wypisz w konsoli informację przy każdym przełączeniu dzień/noc"

**Plik**: [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp) linia ~199-202

```diff
  if (keyN && !prevKeyN_) {
    Lighting& lit = scene_.lighting();
    lit.setMode(lit.mode() == LightingMode::Day ? LightingMode::Night : LightingMode::Day);
+   std::printf("Tryb: %s\n", lit.mode() == LightingMode::Day ? "DZIEN" : "NOC");
    updateWindowTitle();
  }
```

---

### Zadanie 26: „Zmień częstotliwość umieszczania lamp wzdłuż krawędzi parkingu (np. co 3 miejsca zamiast co 5)"

**Plik**: [ParkingScene.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingScene.cpp) linia ~240

```diff
- constexpr int kLampEveryNSpots = 5;
+ constexpr int kLampEveryNSpots = 3;
```

---

### Zadanie 27: „Zmień rozmiar shadow mapy (lepsze/gorsze cienie)"

**Plik**: [Renderer.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.hpp) linia ~65

```diff
- int shadowMapSize_{2048};
+ int shadowMapSize_{4096};   // lepsze cienie, wolniejsze
```

Lub `1024` — gorsze cienie, szybciej.

---

### Zadanie 28: „Zmień model 'specular' — zrób samochody bardziej błyszczące"

**Plik**: [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp) linia ~1007-1008

```diff
    if (p.kind == PropKind::Car) {
-     spec = 0.16f;
+     spec = 0.55f;
    }
```

**Wyjaśnienie**: `spec` (specular strength) kontroluje siłę odbłysków. Wyższy = bardziej „lakierowany".

---

## 📕 POZIOM TRUDNY — dodanie nowej funkcji / modyfikacja logiki

---

### Zadanie 29: „Dodaj drzewa wokół parkingu"

Krok 1 — Dodaj nowy `PropKind` w [ParkingScene.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingScene.hpp) linia ~14:
```diff
- enum class PropKind : uint8_t { Car, Lamp };
+ enum class PropKind : uint8_t { Car, Lamp, Tree };
```

Krok 2 — Dodaj pole w [Renderer.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.hpp):
```diff
  GltfModel lampModel_{};
+ GltfModel treeModel_{};
```

Krok 3 — Załaduj model w [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp) `init()` po linii ~461:
```diff
  lampModel_.loadFromFile(assetPath("models/scifi_lamp.glb"), 2.8f);
+ treeModel_.loadFromFile(assetPath("models/pine_tree_game-ready.glb"), 6.0f);
```

Krok 4 — Umieść drzewa w [ParkingScene.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingScene.cpp) `rebuildPlacements()` (na końcu, przed `}`):
```cpp
  // Drzewa wokół parkingu
  constexpr float kTreeScale = 1.0f;
  const float treeMargin = 6.0f;
  for (int i = 0; i < spotsPerRow; i += 4) {
    const float tx = -halfL + (static_cast<float>(i) + 0.5f) * dx;
    props_.push_back({PropKind::Tree, {tx, 0.0f, -halfW - treeMargin}, 0.0f, kTreeScale});
    props_.push_back({PropKind::Tree, {tx, 0.0f, halfW + treeMargin}, 0.0f, kTreeScale});
  }
```

Krok 5 — Dodaj rysowanie drzew w [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp), w pętli `for (const PlacedProp& p : scene.props())` (linia ~993-1005):
```diff
    } else if (p.kind == PropKind::Lamp && lampModel_.ready()) {
      model = &lampModel_;
+   } else if (p.kind == PropKind::Tree && treeModel_.ready()) {
+     model = &treeModel_;
    }
```

**To samo w `renderShadowPass()`** (linia ~356-358):
```diff
    } else if (p.kind == PropKind::Lamp && lampModel_.ready()) {
      model = &lampModel_;
+   } else if (p.kind == PropKind::Tree && treeModel_.ready()) {
+     model = &treeModel_;
    }
```

---

### Zadanie 30: „Dodaj tryb 'widok z góry' — klawisz T ustawia kamerę na 90° (plan view)"

**Plik**: [Input.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/input/Input.cpp):
```diff
+   case GLFW_KEY_T:
+     if (h->camera) {
+       h->camera->setOrbit(h->camera->distance(), 0.0f, 88.0f, glm::vec3(0.0f));
+     }
+     break;
```

Dodaj include:
```diff
+ #include "rendering/Camera.hpp"
```

**Wyjaśnienie**: pitch 88° = prawie idealnie z góry (90° dałoby gimbal lock). yaw 0° = północ na górze.

---

### Zadanie 31: „Zrób, żeby lampy nie świeciły w dzień (model widoczny, ale bez światła) — a teraz odwróć: niech lampy świecą TAKŻE w dzień"

**Plik**: [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp) linia ~808-809

Obecnie lampy świecą TYLKO w nocy:
```cpp
const int lampPointLightCount =
    scene.lighting().mode() == LightingMode::Night ? static_cast<int>(lampPointPos.size()) : 0;
```

Żeby świeciły ZAWSZE:
```diff
- const int lampPointLightCount =
-     scene.lighting().mode() == LightingMode::Night ? static_cast<int>(lampPointPos.size()) : 0;
+ const int lampPointLightCount = static_cast<int>(lampPointPos.size());
```

---

### Zadanie 32: „Zmień algorytm losowego rozmieszczania aut, tak żeby auta były rozmieszczone równomiernie (co drugie miejsce)"

**Plik**: [ParkingScene.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingScene.cpp) linia ~224-235

Zamień losowanie na systematyczne:
```diff
- const uint64_t seed = layoutHash();
- std::seed_seq seq{...};
- std::mt19937 rng(seq);
- std::shuffle(carSpots.begin(), carSpots.end(), rng);
- std::uniform_int_distribution<int> carModelDist(0, 2);
-
- const int placeCount = std::min(targetCars, static_cast<int>(carSpots.size()));
- for (int i = 0; i < placeCount; ++i) {
-   const CarSpot& s = carSpots[static_cast<size_t>(i)];
-   const uint8_t modelIdx = static_cast<uint8_t>(carModelDist(rng));
-   props_.push_back({PropKind::Car, {s.x, kCarY, s.z}, s.yaw, kCarScale, modelIdx});
- }
+ // Co drugie miejsce
+ for (size_t i = 0; i < carSpots.size(); i += 2) {
+   const CarSpot& s = carSpots[i];
+   const uint8_t modelIdx = static_cast<uint8_t>(i % 3);
+   props_.push_back({PropKind::Car, {s.x, kCarY, s.z}, s.yaw, kCarScale, modelIdx});
+ }
```

---

### Zadanie 33: „Dodaj informację tekstową na ekranie (HUD) — np. liczbę miejsc"

**Plik**: [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp)

W metodzie `drawOverlayUi()`, przed `drawHudCornerOverlay(ortho, parkingSettingsOpen);` (~linia 671), dodaj:

```cpp
  // Stały HUD z liczbą miejsc
  {
    char hudText[128];
    std::snprintf(hudText, sizeof(hudText), "Miejsca: %d  Rzedy: %d",
                  gen.spotCount(), gen.rowCount());
    static std::vector<float> hudTriCount;
    hudTriCount.clear();
    constexpr float kHudScale = 1.6f;
    appendEasyFontTriangles(fh, 10.0f, fh - 60.0f, kHudScale, hudText, hudTriCount);
    flushHudTriangles(flatShader_, lineVao_, lineVbo_, ortho, hudTriCount,
                      0.9f, 0.95f, 1.0f, 0.85f);
  }
```

---

### Zadanie 34: „Niech samochody używają tylko jednego modelu (np. tylko AE86)"

**Plik**: [ParkingScene.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingScene.cpp) linia ~233

```diff
-   const uint8_t modelIdx = static_cast<uint8_t>(carModelDist(rng));
+   const uint8_t modelIdx = 0;  // zawsze AE86
```

Wartości: `0` = AE86, `1` = GR86/Xeno GT, `2` = Avalon Hybrid.

---

### Zadanie 35: „Zmień szerokość alejki między rzędami"

**Plik**: [ParkingGenerator.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingGenerator.hpp) linia ~38

```diff
- static constexpr float aisleWidthMeters() { return 8.5f; }
+ static constexpr float aisleWidthMeters() { return 12.0f; }
```

**Wyjaśnienie**: Alejka to pas jezdni między rzędami. 8.5m = standardowe, 12m = szerokie.

---

### Zadanie 36: „Zmień głębokość miejsca parkingowego"

**Plik**: [ParkingGenerator.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingGenerator.hpp) linia ~42

```diff
- static constexpr float spotDepthMeters() { return 13.75f; }
+ static constexpr float spotDepthMeters() { return 10.0f; }
```

---

### Zadanie 37: „Dodaj nowy argument CLI, np. `--night` żeby startować w trybie nocnym"

**Plik**: [main.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/main.cpp)

Dodaj po linii ~107 (przed `try`), nową zmienną i parsowanie:

```diff
+   bool startNight = false;
    for (int i = 1; i < argc; ++i) {
      const char* a = argv[i];
      // ... istniejące argumenty ...
+     if (std::strcmp(a, "--night") == 0) {
+       startNight = true;
+       continue;
+     }
    }

    try {
      parking::ParkingScene scene(std::move(gen));
+     if (startNight) {
+       scene.lighting().setMode(parking::LightingMode::Night);
+     }
      parking::Application app(std::move(scene));
```

Dodaj include:
```diff
+ #include "lighting/Lighting.hpp"
```

---

## 📚 PYTANIA TEORETYCZNE (prowadzący może zapytać ustnie)

---

### P1: „Co to jest shadow mapping i jak działa w tym projekcie?"

**Odpowiedź**: Shadow mapping to dwu-przebiegowa technika renderowania cieni:
1. **Shadow pass** — renderujemy scenę z punktu widzenia światła (słońca) do tekstury głębokości (`shadowTex_`, 2048×2048). Zapisujemy tylko odległość każdego fragmentu od światła.
2. **Main pass** — dla każdego piksela na ekranie sprawdzamy, czy jego pozycja w „przestrzeni światła" jest dalej niż wartość zapisana w shadow mapie. Jeśli tak → jest w cieniu.

W kodzie:
- `initShadowMap()` tworzy FBO + teksturę głębokości
- `renderShadowPass()` rysuje modele i trawę do shadow mapy
- W shaderach (`model.frag`, `ground.frag`) funkcja `shadowFactor()` próbkuje shadow mapę z **PCF** (3×3 kernel) dla miękkich krawędzi cieni.

---

### P2: „Co to jest VAO, VBO i EBO?"

**Odpowiedź**:
- **VBO** (Vertex Buffer Object) — bufor na GPU z danymi wierzchołków (pozycje x,y,z, normalne, UV). Przykład: `quadVbo_` zawiera 4 wierzchołki prostokąta.
- **EBO** (Element Buffer Object) — bufor z indeksami. Zamiast powtarzać wierzchołki, podajemy kolejność: {0,1,2, 2,3,0} = dwa trójkąty z 4 wierzchołków.
- **VAO** (Vertex Array Object) — „konfiguracja" mówiąca GPU jak interpretować dane z VBO. Np. `glVertexAttribPointer(0, 3, GL_FLOAT, ...)` = „atrybut 0 to 3 floaty (x,y,z) co 12 bajtów".

---

### P3: „Czym jest uniform w shaderze?"

**Odpowiedź**: Uniform to zmienna globalna w shaderze, ustawiana z kodu C++ na CPU. Jest stała dla wszystkich wierzchołków/pikseli w jednym wywołaniu draw. Przykład:
```cpp
// C++:
flatShader_.setVec3("uColor", 0.92f, 0.92f, 0.85f);
// GLSL:
uniform vec3 uColor;
```
Używamy uniformów do przekazywania: macierzy transformacji, kolorów, pozycji światła, flag (dzień/noc), itd.

---

### P4: „Jak działa Phong lighting w tym projekcie?"

**Odpowiedź**: Model oświetlenia Phong składa się z trzech składowych:
1. **Ambient** = minimalna jasność (stała, niezależna od kierunku) → `uAmbient`
2. **Diffuse** = jasność zależna od kąta między normalną a kierunkiem światła → `max(dot(n, L), 0.0)` 
3. **Specular** = odbłysk — jasna plamka na błyszczących powierzchniach → `pow(max(dot(n, H), 0.0), 40.0)`, gdzie H = half-vector (między kierunkiem światła a kamery)

W `model.frag`:
```glsl
float dirLight = uAmbient + (1.0 - uAmbient) * nd * sh;  // ambient + diffuse
float spec = pow(max(dot(n, H), 0.0), 40.0) * uSpecularStrength;  // specular
```

---

### P5: „Co robi macierz MVP? Z czego się składa?"

**Odpowiedź**: MVP = Model × View × Projection. Przekształca pozycję wierzchołka z przestrzeni modelu na ekran:
- **Model** — pozycja obiektu w świecie (translate, rotate, scale)
- **View** — „odwrotność" pozycji kamery (przesuwa świat tak, jakby kamera była w (0,0,0))
- **Projection** — perspektywa (obiekty dalej = mniejsze). W tym projekcie: `glm::perspective(fov, aspect, near, far)`

W kodzie kamera zwraca `camera.view()` i `camera.projection(aspect)`, a model jest budowany per-obiekt:
```cpp
glm::mat4 m = glm::translate(glm::mat4(1.0f), p.position);
m = glm::rotate(m, p.rotY, glm::vec3(0.0f, 1.0f, 0.0f));
m = glm::scale(m, glm::vec3(p.scale));
```

---

### P6: „Jak działa instancing trawy?"

**Odpowiedź**: Zamiast rysować każde źdźbło osobno (byłoby za wolno — setki tysięcy), używamy **geometry instancing**. Tworzymy **jeden** kształt źdźbła (dwa krzyżujące się quady) i rysujemy go tysiące razy jednym wywołaniem `glDrawElementsInstanced()`. Każda instancja ma swoją pozycję i obrót, przekazane przez osobny VBO (`instanceVbo_`). Shader `grass_inst.vert` dodaje animację wiatru (sinusoida zależna od czasu).

---

### P7: „Jaka jest różnica między vertex a fragment shaderem?"

**Odpowiedź**:
- **Vertex shader** — uruchamiany raz per wierzchołek. Oblicza pozycję na ekranie (`gl_Position`). Może też przekazać dane do fragment shadera (normalne, UV, pozycja w świecie).
- **Fragment shader** — uruchamiany raz per piksel (fragment). Oblicza końcowy kolor piksela (`FragColor`). Tu dzieje się oświetlenie, cienie, teksturowanie.

---

### P8: „Po co jest biblioteka Assimp?"

**Odpowiedź**: Assimp (Asset Import Library) wczytuje modele 3D z różnych formatów (glTF, FBX, OBJ itd.). W tym projekcie ładuje pliki `.glb` (binarny glTF) — samochody i lampy. Assimp parsuje geometrię (wierzchołki, normalne, UV), materiały (kolory, tekstury) i zwraca je w ujednoliconej strukturze, z której `GltfModel::loadFromFile()` tworzy bufory OpenGL (VAO/VBO/EBO).

---

### P9: „Jak działają chmury na niebie?"

**Odpowiedź**: Chmury w `sky.frag` są w pełni proceduralne (nie ma żadnej tekstury). Używają **Fractional Brownian Motion (FBM)** — sumowania kilku warstw szumu (value noise) w różnych skalach. Kierunek patrzenia jest rzutowany na „kopułę" chmur (UV = xz/y), z dryfem zależnym od czasu (`uTime`). Funkcja `cloudCoverage()` zwraca 0–1 (ile chmury), a w `daySky()` kolor nieba jest mieszany z kolorem chmury w zależności od pokrycia.

---

### P10: „Co to jest PCF w shadow mappingu?"

**Odpowiedź**: PCF = Percentage Closer Filtering. Zamiast próbkować shadow mapę w jednym punkcie (daje schodkowe krawędzie cieni), próbkujemy w siatce 3×3 wokół aktualnego piksela. Każda próbka daje „0.35 (cień) lub 1.0 (światło)", a wynik uśredniamy. To daje miękkie, rozmyte krawędzie cieni. W kodzie (np. `model.frag:45-51`):
```glsl
for (int x = -1; x <= 1; ++x) {
    for (int y = -1; y <= 1; ++y) {
        float d = texture(uShadowMap, ...);
        s += (cur - bias > d) ? 0.35 : 1.0;
    }
}
return s / 9.0;
```

---

> [!TIP]
> **Strategia na obronę**: 
> 1. Przed obroną odpal projekt i poklikaj — pokaż że wiesz jak działa interaktywnie
> 2. Na pytanie praktyczne ("zmień X") — otwórz właściwy plik i zmień wartość
> 3. Na pytanie teoretyczne — odpowiedz krótko, podaj analogię, wskaż linię w kodzie
> 4. Jeśli nie wiesz — powiedz "muszę sprawdzić w kodzie" i szukaj po nazwach uniformów (Ctrl+F)
