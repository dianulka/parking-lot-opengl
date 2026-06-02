# 🎯 Zadania pogrupowane tematycznie — pełne rozwiązania

> [!IMPORTANT]
> Każda sekcja = jeden element wizualny. Szukaj sekcji pasującej do pytania prowadzącego.

---

## 🌿 TRAWA (źdźbła 3D + tekstura podłoża)

Trawa składa się z **dwóch elementów**:
1. **Źdźbła 3D** — instancingowe quady w [GrassBlades.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/models/GrassBlades.cpp) + shader [grass_inst.vert](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/assets/shaders/grass_inst.vert) / [grass_inst.frag](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/assets/shaders/grass_inst.frag)
2. **Tekstura podłoża** — proceduralny kolor trawy w [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp) (`fillGrassRgba()`) + shader [ground.frag](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/assets/shaders/ground.frag)

---

### 🌿1: „Niech trawa macha szybciej (szybszy wiatr)"

**Plik**: [GrassBlades.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/models/GrassBlades.cpp) linia ~225

```diff
- constexpr float kWindFreq = 2.35f;
+ constexpr float kWindFreq = 8.0f;    // 3× szybciej
```

**Wyjaśnienie**: `kWindFreq` to częstotliwość sinusoidy wiatru. Wyższa = szybsze machanie. Oryginał 2.35, spróbuj 5–10 dla widocznego efektu.

---

### 🌿2: „Niech trawa macha mocniej (większa amplituda wiatru)"

**Plik**: [GrassBlades.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/models/GrassBlades.cpp) linia ~226

```diff
- constexpr float kWindAmpM = 0.25f;
+ constexpr float kWindAmpM = 0.8f;    // 3× większe wychylenie
```

**Wyjaśnienie**: `kWindAmpM` = amplituda wychylenia czubka źdźbła w metrach. 0.25 = subtelne, 0.8 = spora burza.

---

### 🌿3: „Zatrzymaj ruch trawy (zero wiatru)"

**Plik**: [GrassBlades.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/models/GrassBlades.cpp) linia ~226

```diff
- constexpr float kWindAmpM = 0.25f;
+ constexpr float kWindAmpM = 0.0f;
```

---

### 🌿4: „Usuń źdźbła trawy 3D (zostaw tylko płaską teksturę)"

**Plik**: [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp) linia ~914-922

Zakomentuj lub usuń cały blok rysowania trawy:

```diff
- if (grassBlades_.ready()) {
-   glActiveTexture(GL_TEXTURE0 + kShadowUnit);
-   glBindTexture(GL_TEXTURE_2D, shadowTex_);
-   grassBlades_.draw(vp, lightVP, grassShader_, lightDir, amb, kShadowUnit, timeSec, directionalWeight, sunColor,
-                     sunDiskWorldPos, sunDiskWeight, kSunDiskIntensity, kSunDiskRadius, sunColor,
-                     lampPointLightCount,
-                     lampPointLightCount > 0 ? lampPointPos.data() : nullptr, pointIntensity, kPointRadius,
-                     glm::vec3(1.0f, 0.93f, 0.72f), grassPointLightScale);
- }
+ // Źdźbła trawy wyłączone
```

Tak samo w `renderShadowPass()` (~342-344):
```diff
- if (grassBlades_.ready()) {
-   grassBlades_.drawShadow(lightViewProj, grassDepthShader_, timeSec);
- }
+ // Źdźbła trawy wyłączone w cieniach
```

---

### 🌿5: „Zmień kolor źdźbeł trawy (np. jesienne — brązowo-żółte)"

**Plik**: [grass_inst.frag](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/assets/shaders/grass_inst.frag) linia ~53-55

```diff
- vec3 baseDark = vec3(0.12, 0.38, 0.1);
- vec3 baseLight = vec3(0.22, 0.62, 0.18);
+ vec3 baseDark = vec3(0.35, 0.25, 0.08);    // ciemny brąz
+ vec3 baseLight = vec3(0.65, 0.55, 0.15);   // żółty
```

**Wyjaśnienie**: `baseDark` = kolor u nasady źdźbła, `baseLight` = kolor czubka. `mix(baseDark, baseLight, vTip)` interpoluje między nimi. Zmiana tych dwóch linii daje efekt jesiennej trawy.

---

### 🌿6: „Zmień kolor tekstury podłoża trawy (flat ground)"

**Plik**: [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp) linia ~872

```diff
- groundShader_.setVec3("uGrassTint", 0.55f, 0.95f, 0.48f);
+ groundShader_.setVec3("uGrassTint", 0.75f, 0.65f, 0.25f);  // sucha trawa
```

---

### 🌿7: „Zmień wysokość źdźbeł trawy"

**Plik**: [GrassBlades.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/models/GrassBlades.cpp) linia ~61

```diff
- constexpr float kClumpHeightM = 1.10f;
+ constexpr float kClumpHeightM = 0.4f;   // niska trawa
```

I w shaderze [grass_inst.vert](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/assets/shaders/grass_inst.vert) linia ~12 (musi się zgadzać!):
```diff
- const float kBladeH = 1.10;
+ const float kBladeH = 0.4;
```

> [!WARNING]
> Te dwie wartości **MUSZĄ** być takie same — jedna w C++, druga w shaderze. Jeśli się rozjadą, animacja wiatru będzie nieproporcjonalna.

---

### 🌿8: „Zmień gęstość trawy (więcej / mniej źdźbeł)"

**Plik**: [GrassBlades.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/models/GrassBlades.cpp) linia ~139

```diff
- constexpr int kBladesBudgetPerTuftAnchor = 54;
+ constexpr int kBladesBudgetPerTuftAnchor = 120;  // dużo gęściej (wolniej!)
```

Lub mniej:
```diff
+ constexpr int kBladesBudgetPerTuftAnchor = 15;   // rzadko
```

Alternatywnie zmień maksymalną liczbę instancji (linia ~135):
```diff
- constexpr int kMaxInstances = 240000;
+ constexpr int kMaxInstances = 80000;   // mniej = szybciej ale rzadziej
```

---

### 🌿9: „Zmień szerokość źdźbeł trawy (grubsze / cieńsze)"

**Plik**: [GrassBlades.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/models/GrassBlades.cpp) linia ~62

```diff
- constexpr float hw = 0.25f;
+ constexpr float hw = 0.08f;   // cienkie źdźbła
```

Lub:
```diff
+ constexpr float hw = 0.5f;    // szerokie łopatki
```

---

### 🌿10: „Zmień kierunek wiatru (trawa się wychyla w inną stronę)"

**Plik**: [grass_inst.vert](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/assets/shaders/grass_inst.vert) linia ~27-28

```diff
- float swayX = sin(ph) * uWindAmp * bend + sin(ph2) * uWindAmp * 0.28 * bend;
- float swayZ = sin(ph * 0.73 + 1.9) * uWindAmp * 0.62 * bend;
+ float swayX = sin(ph) * uWindAmp * 0.3 * bend;     // mało w X
+ float swayZ = sin(ph * 0.73 + 1.9) * uWindAmp * 1.5 * bend;  // dużo w Z
```

**Wyjaśnienie**: `swayX` i `swayZ` to wychylenie w osiach X i Z. Zwiększ/zmniejsz mnożniki żeby zmienić kierunek dominujący wiatru.

---

### 🌿11: „Zmień rozmiar marginesu trawnika wokół parkingu"

**Plik**: [ParkingGenerator.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingGenerator.hpp) linia ~48

```diff
- static constexpr float grassMarginMeters() { return 88.0f; }
+ static constexpr float grassMarginMeters() { return 40.0f; }  // mniej trawy
```

---

### 🌿12: „Zmień rozrzut kępek trawy (bardziej / mniej równomiernie)"

**Plik**: [GrassBlades.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/models/GrassBlades.cpp) linia ~141

```diff
- constexpr float kMiniClumpScatter = 2.35f;
+ constexpr float kMiniClumpScatter = 5.0f;   // kępki bardziej rozproszone
```

---

## 🔦 LAMPY

Lampy to: model 3D (`scifi_lamp.glb`) + światła punktowe w nocy.

Logika rozmieszczenia: [ParkingScene.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingScene.cpp) `rebuildPlacements()`
Renderowanie: [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp)
Ładowanie modelu: [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp) `init()` linia ~461

---

### 🔦1: „Zmień skalę lamp (większe / mniejsze)"

**Plik**: [ParkingScene.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingScene.cpp) linia ~237

```diff
- constexpr float kLampScale = 0.93f;
+ constexpr float kLampScale = 1.5f;    // 60% większe
```

---

### 🔦2: „Zmień zasięg światła lamp (świecą dalej / bliżej)"

**Plik**: [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp) linia ~810

```diff
- constexpr float kPointRadius = 118.0f;
+ constexpr float kPointRadius = 200.0f;    // dużo dalej
```

Lub mniejszy zasięg:
```diff
+ constexpr float kPointRadius = 40.0f;     // tylko blisko lampy
```

---

### 🔦3: „Zmień kolor światła lamp (np. ciepłe żółte → zimne białe)"

**Plik**: [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp) — trzeba zmienić we **wszystkich** `setVec3("uPointColor", ...)`:

Linia ~891:
```diff
- groundShader_.setVec3("uPointColor", 1.0f, 0.93f, 0.72f);
+ groundShader_.setVec3("uPointColor", 0.95f, 0.97f, 1.0f);   // zimne białe
```

Linia ~921 (parametr do `grassBlades_.draw`):
```diff
- glm::vec3(1.0f, 0.93f, 0.72f));
+ glm::vec3(0.95f, 0.97f, 1.0f));
```

Linia ~982:
```diff
- modelShader_.setVec3("uPointColor", 1.0f, 0.93f, 0.72f);
+ modelShader_.setVec3("uPointColor", 0.95f, 0.97f, 1.0f);
```

---

### 🔦4: „Zmień intensywność lamp (jaśniej / ciemniej)"

**Plik**: [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp) linia ~803

```diff
- constexpr float kLampPointIntensityScale = 0.13f;
+ constexpr float kLampPointIntensityScale = 0.30f;   // 2× jaśniej
```

---

### 🔦5: „Niech lampy świecą też w dzień (nie tylko w nocy)"

**Plik**: [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp) linia ~808-809

```diff
- const int lampPointLightCount =
-     scene.lighting().mode() == LightingMode::Night ? static_cast<int>(lampPointPos.size()) : 0;
+ const int lampPointLightCount = static_cast<int>(lampPointPos.size());
```

---

### 🔦6: „Zmień co ile miejsc stoi lampa (rzadziej / gęściej)"

**Plik**: [ParkingScene.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingScene.cpp) linia ~240

```diff
- constexpr int kLampEveryNSpots = 5;
+ constexpr int kLampEveryNSpots = 2;    // lampa co 2 miejsca
```

---

### 🔦7: „Usuń lampy z drogi dojazdowej (zostaw tylko przy parkingu)"

**Plik**: [ParkingScene.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingScene.cpp) linia ~258-270

Zakomentuj cały blok lamp drogowych:
```diff
- // Lampy wzdłuż drogi dojazdowej do parkingu (poza obrysem)
- constexpr float kRoadLampAlongStepM = 100.0f;
- constexpr float kRoadLampEdgeMargin = 12.0f;
- const float roadXMin = -gHalfL + kRoadLampEdgeMargin;
- const float roadXMax = gHalfL - kRoadLampEdgeMargin;
- const float parkEdgeLeft = -halfL - 4.0f;
- const float parkEdgeRight = halfL + 4.0f;
- for (float x = roadXMin; x < parkEdgeLeft; x += kRoadLampAlongStepM) {
-   props_.push_back({PropKind::Lamp, {x, 0.0f, 0.0f}, 0.0f, kLampScale});
- }
- for (float x = parkEdgeRight; x < roadXMax; x += kRoadLampAlongStepM) {
-   props_.push_back({PropKind::Lamp, {x, 0.0f, 0.0f}, kPi, kLampScale});
- }
+ // Lampy drogowe usunięte
```

---

### 🔦8: „Zmień pozycję żarówki w modelu lampy (przesunie źródło światła)"

**Plik**: [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp) linia ~781

```diff
- constexpr glm::vec3 kLampBulbLocal(0.1f, 2.42f, 0.36f);
+ constexpr glm::vec3 kLampBulbLocal(0.0f, 3.5f, 0.0f);    // wyżej, na środku
```

**Wyjaśnienie**: To lokalna pozycja światła w przestrzeni modelu lampy. `y=2.42` = na szczycie lampy. Zmiana tego przesuwa punkt, z którego „świeci" lampa.

---

## 🚗 SAMOCHODY

Modele: 3× `.glb` w `assets/models/`
Rozmieszczenie: [ParkingScene.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingScene.cpp) `rebuildPlacements()`
Ładowanie: [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp) `init()` linia ~452-460

---

### 🚗1: „Zmień wielkość samochodów"

**Plik**: [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp) linia ~457

```diff
- constexpr float kCarNormalizeM = 4.5f;
+ constexpr float kCarNormalizeM = 3.0f;   // małe autka
```

Lub:
```diff
+ constexpr float kCarNormalizeM = 6.0f;   // SUV-y
```

---

### 🚗2: „Użyj tylko jednego modelu samochodu (np. tylko Avalon)"

**Plik**: [ParkingScene.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingScene.cpp) linia ~233

```diff
- const uint8_t modelIdx = static_cast<uint8_t>(carModelDist(rng));
+ const uint8_t modelIdx = 2;   // 0=AE86, 1=GR86, 2=Avalon
```

---

### 🚗3: „Zmień procent zajętych miejsc"

**Plik**: [ParkingScene.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingScene.cpp) linia ~201

```diff
- int targetCars = (totalSpots * 2) / 5;    // 40%
+ int targetCars = (totalSpots * 4) / 5;    // 80%
```

---

### 🚗4: „Usuń limit 20 aut na scenie"

**Plik**: [ParkingScene.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingScene.cpp) linia ~203

```diff
- const int maxCars = std::min(20, std::max(1, totalSpots - 1));
+ const int maxCars = std::max(1, totalSpots - 1);   // bez limitu
```

---

### 🚗5: „Zmień wysokość na jakiej stoją samochody (np. lewitujące auta)"

**Plik**: [ParkingScene.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingScene.cpp) linia ~17

```diff
- constexpr float kCarY = 0.06f;
+ constexpr float kCarY = 2.0f;    // lewitują 2m nad ziemią!
```

---

### 🚗6: „Zrób auta mniej / bardziej błyszczące"

**Plik**: [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp) linia ~1008

```diff
      spec = 0.16f;
```

Zmień na:
- `0.0f` = matowe (brak odbłysków)
- `0.5f` = mocno błyszczące
- `1.0f` = lustro

---

### 🚗7: „Zmień rozmiar hitboxu do klikania na auto (łatwiej / trudniej trafić)"

**Plik**: [ParkingScene.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingScene.cpp) linia ~22

```diff
- const glm::vec3 kCarPickHalfExtents(2.15f, 0.52f, 1.0f);
+ const glm::vec3 kCarPickHalfExtents(3.5f, 1.0f, 2.0f);   // dużo łatwiej trafić
```

---

### 🚗8: „Zmień minimalną odległość między autami (pozwól na ciasne parkowanie)"

**Plik**: [ParkingScene.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingScene.cpp) linia ~323

```diff
- const float sep = g.dx * 0.35f;
+ const float sep = g.dx * 0.15f;    // ciasno!
```

---

## ☁️ NIEBO

Niebo jest w pełni proceduralne — shader [sky.frag](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/assets/shaders/sky.frag).
Dzień: gradient + słońce + chmury. Noc: gradient + gwiazdy + księżyc.

---

### ☁️1: „Przyspiesz ruch chmur"

**Plik**: [sky.frag](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/assets/shaders/sky.frag) linia ~52

```diff
- vec2 drift = vec2(timeSec * 0.0040, timeSec * 0.0026);
+ vec2 drift = vec2(timeSec * 0.025, timeSec * 0.018);   // 6× szybciej
```

---

### ☁️2: „Wyłącz chmury (czyste niebo)"

**Plik**: [sky.frag](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/assets/shaders/sky.frag) linia ~96-107

```diff
- float cov = cloudCoverage(d, timeSec);
- if (cov > 0.0) {
-   // ... cały blok chmur ...
- }
+ // Chmury wyłączone
```

---

### ☁️3: „Więcej chmur na niebie (większe pokrycie)"

**Plik**: [sky.frag](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/assets/shaders/sky.frag) linia ~58

```diff
- float cumulus = smoothstep(0.46, 0.82, baseShape);
+ float cumulus = smoothstep(0.25, 0.65, baseShape);   // niższy próg = więcej chmur
```

---

### ☁️4: „Zmień kolor nieba dziennego (gradient zenit↔horyzont)"

**Plik**: [sky.frag](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/assets/shaders/sky.frag) linia ~73-75

```diff
- vec3 zenith   = vec3(0.16, 0.42, 0.86);    // niebieski zenit
- vec3 horizon  = vec3(0.82, 0.90, 0.99);    // jasny horyzont
+ vec3 zenith   = vec3(0.05, 0.15, 0.55);    // ciemniejszy
+ vec3 horizon  = vec3(0.90, 0.70, 0.50);    // ciepły pomarańcz (zachód)
```

---

### ☁️5: „Powiększ dysk słońca"

**Plik**: [sky.frag](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/assets/shaders/sky.frag) linia ~91

```diff
- float disk = smoothstep(0.9988, 0.99975, cs);
+ float disk = smoothstep(0.995, 0.999, cs);   // 5× większy dysk
```

---

### ☁️6: „Więcej gwiazd w nocy"

**Plik**: [sky.frag](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/assets/shaders/sky.frag) linia ~130-131

```diff
- float threshold = (layer == 0) ? 0.9968 : 0.9988;
+ float threshold = (layer == 0) ? 0.990 : 0.995;   // dużo więcej gwiazd
```

**Wyjaśnienie**: Niższy próg = więcej gwiazd przejdzie filtr hash. 0.9968 → ~0.3% pikseli to gwiazdy. 0.990 → ~1% = 3× więcej.

---

### ☁️7: „Zmień kolor księżyca"

**Plik**: [sky.frag](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/assets/shaders/sky.frag) linia ~151

```diff
- vec3 moonCol = vec3(0.86, 0.88, 0.95);
+ vec3 moonCol = vec3(1.0, 0.85, 0.5);   // złoty księżyc
```

---

### ☁️8: „Zmień kolor nocnego nieba (np. ciemnofioletowe)"

**Plik**: [sky.frag](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/assets/shaders/sky.frag) linia ~116-118

```diff
- vec3 zenith   = vec3(0.003, 0.006, 0.018);
- vec3 horizon  = vec3(0.028, 0.044, 0.100);
+ vec3 zenith   = vec3(0.02, 0.005, 0.04);    // fioletowy zenit
+ vec3 horizon  = vec3(0.06, 0.02, 0.10);     // fioletowy horyzont
```

---

## 🅿️ NAWIERZCHNIA PARKINGU I LINIE

Nawierzchnia: [ground.frag](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/assets/shaders/ground.frag) + [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp)
Linie: `buildParkingLines()` w [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp) + flat shader

---

### 🅿️1: „Zmień kolor linii parkingowych"

**Plik**: [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp) linia ~940

```diff
- flatShader_.setVec3("uColor", 0.92f, 0.92f, 0.85f);
+ flatShader_.setVec3("uColor", 1.0f, 1.0f, 0.0f);     // żółte
```

---

### 🅿️2: „Zmień kolor osi jezdni (środkowa linia)"

**Plik**: [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp) linia ~959

```diff
- flatShader_.setVec3("uColor", 0.92f, 0.82f, 0.18f);
+ flatShader_.setVec3("uColor", 1.0f, 0.0f, 0.0f);     // czerwona
```

---

### 🅿️3: „Zmień kolor nawierzchni parkingu"

**Plik**: [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp) linia ~874

```diff
- groundShader_.setVec3("uParkingTint", 0.62f, 0.62f, 0.64f);
+ groundShader_.setVec3("uParkingTint", 0.40f, 0.40f, 0.45f);  // ciemny asfalt
```

---

### 🅿️4: „Zmień kolor drogi dojazdowej"

**Plik**: [Renderer.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.cpp) linia ~873

```diff
- groundShader_.setVec3("uRoadStripTint", 0.58f, 0.58f, 0.6f);
+ groundShader_.setVec3("uRoadStripTint", 0.30f, 0.30f, 0.35f);  // ciemna droga
```

---

### 🅿️5: „Zmień szerokość alejki między rzędami"

**Plik**: [ParkingGenerator.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingGenerator.hpp) linia ~38

```diff
- static constexpr float aisleWidthMeters() { return 8.5f; }
+ static constexpr float aisleWidthMeters() { return 14.0f; }   // szeroka alejka
```

---

### 🅿️6: „Zmień głębokość miejsc parkingowych"

**Plik**: [ParkingGenerator.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingGenerator.hpp) linia ~42

```diff
- static constexpr float spotDepthMeters() { return 13.75f; }
+ static constexpr float spotDepthMeters() { return 8.0f; }    // krótkie miejsca
```

---

### 🅿️7: „Zmień minimalną szerokość stanowiska"

**Plik**: [ParkingGenerator.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingGenerator.hpp) linia ~45

```diff
- static constexpr float minSpotWidthAlongLotMeters() { return 4.0f; }
+ static constexpr float minSpotWidthAlongLotMeters() { return 2.5f; }   // ciasne stanowiska
```

---

### 🅿️8: „Zmień margines asfaltu wokół parkingu"

**Plik**: [ParkingGenerator.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/scene/ParkingGenerator.hpp) linia ~51

```diff
- static constexpr float pavedLotMarginMeters() { return 26.0f; }
+ static constexpr float pavedLotMarginMeters() { return 5.0f; }    // mały margines
```

---

## 💡 OŚWIETLENIE (dzień / noc)

Wszystko w: [Lighting.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/lighting/Lighting.cpp) (35 linii!)

---

### 💡1: „Zmień jasność sceny dziennej (ambient)"

**Plik**: [Lighting.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/lighting/Lighting.cpp) linia ~16

```diff
- return 0.42f;
+ return 0.70f;    // prawie bez cieni
```

---

### 💡2: „Dodaj ambient w nocy (nie kompletna ciemność)"

**Plik**: [Lighting.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/lighting/Lighting.cpp) linia ~14

```diff
- return 0.0f;
+ return 0.08f;    // lekka poświata nocna
```

---

### 💡3: „Zmień kierunek słońca (cienie w inną stronę)"

**Plik**: [Lighting.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/lighting/Lighting.cpp) linia ~31

```diff
- return glm::normalize(glm::vec3(0.35f, 0.85f, 0.45f));
+ return glm::normalize(glm::vec3(-0.7f, 0.5f, -0.3f));   // z zachodu, nisko
```

**Wyjaśnienie**: Wektor (x, y, z) wskazuje **ku** słońcu. `y` = wysokość: 0.85 = wysoko, 0.3 = nisko. Zmiana x/z = zmiana azymutu.

---

### 💡4: „Zmień kolor słońca (cieplejsze / zimniejsze)"

**Plik**: [Lighting.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/lighting/Lighting.cpp) linia ~24

```diff
- return glm::vec3(1.0f, 0.993f, 0.978f);
+ return glm::vec3(1.0f, 0.85f, 0.6f);    // ciepłe pomarańczowe
```

---

### 💡5: „Zmień domyślny tryb startowy na dzień"

**Plik**: [Lighting.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/lighting/Lighting.hpp) linia ~26

```diff
- LightingMode mode_{LightingMode::Night};
+ LightingMode mode_{LightingMode::Day};
```

---

### 💡6: „Zmień kolor tła (clear color) nieba"

**Plik**: [Lighting.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/lighting/Lighting.cpp) linia ~7 i ~9

Noc:
```diff
- return glm::vec3(0.0f, 0.0f, 0.0f);        // czarny
+ return glm::vec3(0.01f, 0.01f, 0.05f);      // ciemny granat
```

Dzień:
```diff
- return glm::vec3(0.55f, 0.68f, 0.88f);      // niebieski
+ return glm::vec3(0.85f, 0.55f, 0.30f);      // zachód słońca
```

---

## 📐 KAMERA

Kod: [Camera.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Camera.cpp) + [Camera.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Camera.hpp)
Sterowanie: [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp) `updateCamera()`

---

### 📐1: „Zmień domyślny kąt kamery (np. bardziej z góry)"

**Plik**: [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp) linia ~51-52

```diff
- constexpr float kStartYawDeg = 39.0f;
- constexpr float kStartPitchDeg = 61.5f;
+ constexpr float kStartYawDeg = 0.0f;     // prosto z przodu
+ constexpr float kStartPitchDeg = 85.0f;  // prawie z góry
```

---

### 📐2: „Zmień domyślną odległość kamery"

**Plik**: [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp) linia ~50

```diff
- const float startDist = std::max(52.0f, span * 0.46f);
+ const float startDist = std::max(100.0f, span * 0.7f);   // dalej
```

---

### 📐3: „Zmień pole widzenia (FOV)"

**Plik**: [Camera.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Camera.hpp) linia ~38

```diff
- float fovDegrees_{52.0f};
+ float fovDegrees_{75.0f};    // szerokie pole widzenia
```

---

### 📐4: „Zmień limity pochylenia kamery (pozwól na widok z boku)"

**Plik**: [Camera.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Camera.cpp) linia ~43

```diff
- pitchDeg_ = std::clamp(pitchDeg_, 18.0f, 88.0f);
+ pitchDeg_ = std::clamp(pitchDeg_, 3.0f, 88.0f);   // prawie z poziomu gruntu
```

---

### 📐5: „Zmień limity zoom (bliżej / dalej)"

**Plik**: [Camera.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Camera.cpp) linia ~49

```diff
- distance_ = std::clamp(distance_, 8.0f, 420.0f);
+ distance_ = std::clamp(distance_, 3.0f, 800.0f);
```

---

### 📐6: „Zmień prędkość kamery (WASD szybsze, obrót wolniejszy)"

**Plik**: [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp) linia ~22-23

```diff
- constexpr float kCameraPanSpeed = 38.0f;
- constexpr float kCameraOrbitSpeed = 48.0f;
+ constexpr float kCameraPanSpeed = 80.0f;     // szybki pan
+ constexpr float kCameraOrbitSpeed = 20.0f;   // wolny obrót
```

---

## 🖥️ OKNO I OGÓLNE

---

### 🖥️1: „Zmień rozdzielczość okna"

**Plik**: [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp) linia ~30

```diff
- window_(1280, 720, kTitleNormal),
+ window_(1920, 1080, kTitleNormal),
```

---

### 🖥️2: „Zmień tytuł okna"

**Plik**: [Application.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/Application.cpp) linia ~25

```diff
- constexpr char kTitleNormal[] = "Parking prostokątny — OpenGL 3.3";
+ constexpr char kTitleNormal[] = "Moj parking - projekt zaliczeniowy";
```

---

### 🖥️3: „Wyłącz VSync (odblokuj FPS)"

**Plik**: [Window.cpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/core/Window.cpp) linia ~29

```diff
- glfwSwapInterval(1);
+ glfwSwapInterval(0);    // bez VSync, max FPS
```

---

### 🖥️4: „Zmień rozmiar shadow mapy (jakość cieni)"

**Plik**: [Renderer.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/rendering/Renderer.hpp) linia ~65

```diff
- int shadowMapSize_{2048};
+ int shadowMapSize_{4096};    // lepsze cienie (wolniej)
```

Lub `1024` = gorsze cienie, szybciej.

---

### 🖥️5: „Zmień rozmiar przycisku ustawień w rogu ekranu"

**Plik**: [UiConstants.hpp](file:///c:/Users/tonip/Desktop/parking/parking-lot-opengl/src/app/UiConstants.hpp) linia ~6-8

```diff
- constexpr float kCornerPanelPx = 64.0f;
- constexpr float kCornerMarginPx = 8.0f;
+ constexpr float kCornerPanelPx = 100.0f;   // większy przycisk
+ constexpr float kCornerMarginPx = 12.0f;   // dalej od krawędzi
```

---

> [!TIP]
> **Szybki lookup — gdzie szukać według typu zmiany:**
>
> | Zmieniam... | Plik |
> |-------------|------|
> | Kolor czegokolwiek | `Renderer.cpp` (setVec3 "uColor/uTint") lub `.frag` shader |
> | Parametr parkingu (rozmiar, limity) | `ParkingGenerator.hpp` |
> | Ile / gdzie auta/lampy | `ParkingScene.cpp` → `rebuildPlacements()` |
> | Wiatr / trawa 3D | `GrassBlades.cpp` (kWindFreq, kWindAmpM, kClumpHeight) |
> | Kolor trawy 3D | `grass_inst.frag` (baseDark, baseLight) |
> | Niebo / chmury / gwiazdy | `sky.frag` |
> | Kamera (kąt, zoom, prędkość) | `Camera.cpp` / `Camera.hpp` / `Application.cpp` |
> | Oświetlenie dzień/noc | `Lighting.cpp` |
> | Okno / FPS | `Application.cpp` / `Window.cpp` |
