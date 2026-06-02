# Zestaw zadań egzaminacyjnych — projekt `parking-lot-opengl`

> **Forma zaliczenia:** drobna modyfikacja istniejącego zachowania lub dopisanie krótkiego
> fragmentu kodu. Każde zadanie sprawdza, czy rozumiesz architekturę tego konkretnego
> projektu — przepływ zdarzeń (GLFW → `Application` → `ParkingScene`), logikę parkingu oraz
> sposób prezentacji informacji w interfejsie.
>
> Czytaj polecenia dokładnie. Punktowana jest **trafność lokalizacji zmiany** oraz
> **poprawność rozwiązania**, a nie liczba zmienionych linii. Powodzenia.

## Mapa orientacyjna (przed egzaminem przeczytaj)

- `src/main.cpp` — parsowanie argumentów CLI i start aplikacji.
- `src/app/Application.cpp/.hpp` — pętla główna, callbacki GLFW (mysz, scroll, resize),
  obsługa klawiszy panelu ustawień, klik świata, tytuł okna.
- `src/input/Input.cpp` — globalny `keyCallback` (obecnie tylko `ESC`).
- `src/scene/ParkingScene.cpp` — logika sceny: rozmieszczanie aut/lamp, „pick" auta promieniem,
  przenoszenie auta na wolne miejsce wraz z walidacją.
- `src/scene/ParkingGenerator.cpp/.hpp` — parametry parkingu i ich „clampowanie".
- `src/lighting/Lighting.hpp` — tryb dzień/noc.

---

# CZĘŚĆ 1 — Zadania logiczne / biznesowe

## Zadanie 1.1 — Rezerwacja pierwszego rzędu (instrukcja warunkowa)

**Treść polecenia (od prowadzącego):**
> Pierwszy rząd parkingu (rząd o indeksie `0`, najbliższy dolnej krawędzi) zostaje
> zarezerwowany dla pojazdów uprzywilejowanych. Zablokuj możliwość **przenoszenia** auta
> przez użytkownika na jakiekolwiek miejsce w tym rzędzie — próba upuszczenia samochodu
> w rzędzie 0 ma się nie powieść (auto zostaje tam, gdzie było), natomiast pozostałe rzędy
> działają bez zmian. Domyślne rozmieszczenie startowe może nadal stawiać tam auta.

**Pliki do modyfikacji:**
- `src/scene/ParkingScene.cpp` — funkcja `snapHitToSpot(...)` (to ona wybiera docelowy rząd
  podczas przenoszenia auta).

**Rozwiązanie:**

Funkcja `snapHitToSpot` zna już indeks wybranego rzędu (`bestRow`). Wystarczy dodać warunek
odrzucający rząd `0` zaraz po znalezieniu najbliższego rzędu:

```128:148:src/scene/ParkingScene.cpp
bool snapHitToSpot(const LotGeom& g, float hitX, float hitZ, float& outX, float& outZ, float& outYaw) {
  if (g.rowCenterZ.empty()) {
    return false;
  }
  // Wybór najbliższego rzędu po Z; odrzucamy klik na alejce (przerwa między rzędami).
  int bestRow = -1;
  float bestDist = std::numeric_limits<float>::infinity();
  for (int k = 0; k < static_cast<int>(g.rowCenterZ.size()); ++k) {
    const float d = std::abs(hitZ - g.rowCenterZ[static_cast<size_t>(k)]);
    if (d < bestDist) {
      bestDist = d;
      bestRow = k;
    }
  }
  if (bestRow < 0) {
    return false;
  }
```

Dopisujemy po wyznaczeniu `bestRow` (tuż przed sprawdzeniem alejki):

```cpp
  if (bestRow < 0) {
    return false;
  }
  // Rząd 0 jest zarezerwowany — nie pozwalamy tu przenosić aut.
  if (bestRow == 0) {
    return false;
  }
```

`tryMoveCarToWorldXZ` zwraca wtedy `false`, a w `Application::handleWorldLeftClick` auto
pozostaje zaznaczone w starym miejscu — dokładnie takie zachowanie jest oczekiwane.

---

## Zadanie 1.2 — Zmiana ograniczenia liczby zaparkowanych aut (ograniczenie danych)

**Treść polecenia (od prowadzącego):**
> Obecnie scena zapełnia ok. 40% miejsc i nie więcej niż 20 aut. Zmień zasadę biznesową tak,
> aby parking startował **maksymalnie w 25% zapełnienia** oraz aby twardy limit wynosił
> **12 aut** zamiast 20. Pozostałe reguły (minimum 1 auto, nie więcej aut niż dostępnych
> miejsc) zachowaj.

**Pliki do modyfikacji:**
- `src/scene/ParkingScene.cpp` — funkcja `rebuildPlacements(...)`, blok wyliczający `targetCars`.

**Rozwiązanie:**

Oryginalny fragment:

```199:204:src/scene/ParkingScene.cpp
  const int totalSpots = spotsPerRow * rowCount;
  // Cel: ok. 40% miejsc zajętych, ale nie więcej niż 20 aut (limity sceny).
  int targetCars = (totalSpots * 2) / 5;
  targetCars = std::max(1, targetCars);
  const int maxCars = std::min(20, std::max(1, totalSpots - 1));
  targetCars = std::min(targetCars, maxCars);
```

Po zmianie (25% = `/ 4`, limit `12`):

```cpp
  const int totalSpots = spotsPerRow * rowCount;
  // Cel: ok. 25% miejsc zajętych, ale nie więcej niż 12 aut (limity sceny).
  int targetCars = totalSpots / 4;
  targetCars = std::max(1, targetCars);
  const int maxCars = std::min(12, std::max(1, totalSpots - 1));
  targetCars = std::min(targetCars, maxCars);
```

**Uwaga dla zdającego:** komentarz też trzeba poprawić — rozjazd komentarza z kodem jest
błędem merytorycznym. Zmiana wpływa na scenę dopiero przy przebudowie układu (zmiana liczby
rzędów/miejsc), bo `rebuildPlacements` wywoływane jest przez `syncPlacements` na podstawie
hasza układu.

---

## Zadanie 1.3 — Twardsza walidacja kolizji między autami (zmiana walidacji)

**Treść polecenia (od prowadzącego):**
> Klienci skarżą się, że samochody potrafią stać zbyt blisko siebie. Zaostrz regułę kolizji
> przy przenoszeniu auta: zamiast obecnego progu (35% szerokości miejsca) wymagaj, aby
> w docelowym punkcie **nie było innego auta bliżej niż pełna szerokość jednego miejsca**
> (`dx`). Reszta logiki przenoszenia bez zmian.

**Pliki do modyfikacji:**
- `src/scene/ParkingScene.cpp` — funkcja `tryMoveCarToWorldXZ(...)`.

**Rozwiązanie:**

Oryginał:

```323:334:src/scene/ParkingScene.cpp
  const float sep = g.dx * 0.35f;
  const float sep2 = sep * sep;
  for (size_t i = 0; i < props_.size(); ++i) {
    if (props_[i].kind != PropKind::Car || i == propIndex) {
      continue;
    }
    const float dx = props_[i].position.x - sx;
    const float dz = props_[i].position.z - sz;
    if (dx * dx + dz * dz < sep2) {
      return false;
    }
  }
```

Wystarczy zmienić mnożnik separacji z `0.35f` na `1.0f`:

```cpp
  const float sep = g.dx * 1.0f;  // pełna szerokość miejsca jako minimalny odstęp
  const float sep2 = sep * sep;
```

(reszta pętli pozostaje bez zmian).

**Pytanie kontrolne na obronie:** dlaczego porównujemy kwadraty odległości (`sep2`),
a nie pierwiastki? — Aby uniknąć kosztownego `sqrt`; wynik logiczny jest identyczny.

---

# CZĘŚĆ 2 — Zadania zdarzeniowe / interfejsowe

## Zadanie 2.1 — Globalne przełączanie dnia/nocy klawiszem (reakcja na klawisz)

**Treść polecenia (od prowadzącego):**
> Obecnie tryb dzień/noc można przełączyć klawiszem `N` **tylko przy otwartym panelu ustawień**
> (logika w `handleParkingSettingsKeys`). Dodaj globalny skrót: klawisz `L` ma przełączać
> oświetlenie dzień/noc **zawsze**, niezależnie od tego, czy panel jest otwarty. Wykorzystaj
> istniejący globalny `keyCallback`.

**Pliki do modyfikacji:**
- `src/input/Input.cpp` — funkcja `Input::keyCallback`.

**Rozwiązanie:**

`keyCallback` ma już dostęp do sceny przez `h->scene`, a `ParkingScene` udostępnia
`lighting()`. Dodajemy `case` w istniejącym `switch`:

```29:41:src/input/Input.cpp
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
    default:
      break;
  }
```

Po zmianie:

```cpp
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
    case GLFW_KEY_L: {
      Lighting& lit = h->scene->lighting();
      lit.setMode(lit.mode() == LightingMode::Day ? LightingMode::Night : LightingMode::Day);
      break;
    }
    default:
      break;
  }
```

Trzeba też dołączyć nagłówek na górze pliku:

```cpp
#include "lighting/Lighting.hpp"
```

**Pytanie kontrolne:** dlaczego nie użyto tu `glfwGetKey` jak przy `WASD`? — Bo `keyCallback`
jest zdarzeniowy (reaguje na pojedyncze naciśnięcie), więc nie trzeba ręcznie wykrywać zbocza
klawisza jak w `handleParkingSettingsKeys`.

---

## Zadanie 2.2 — Anulowanie wyboru auta prawym przyciskiem myszy (reakcja na klik)

**Treść polecenia (od prowadzącego):**
> Dodaj obsługę **prawego przycisku myszy**: kliknięcie PPM ma anulować aktualny wybór auta
> (jeśli jakieś jest zaznaczone), bez wpływu na panel ustawień ani na kamerę. Lewy przycisk
> działa jak dotychczas.

**Pliki do modyfikacji:**
- `src/app/Application.cpp` — funkcja `mouseButtonCallback`.

**Rozwiązanie:**

Obecnie callback od razu odrzuca wszystko poza LPM:

```226:233:src/app/Application.cpp
void Application::mouseButtonCallback(GLFWwindow* window, int button, int action, int /*mods*/) {
  if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS) {
    return;
  }
  auto* h = static_cast<WindowHooks*>(glfwGetWindowUserPointer(window));
  if (!h || !h->app) {
    return;
  }
```

Dopisujemy obsługę PPM przed dotychczasową logiką. Korzystamy z istniejącej metody
`consumeEscapeForCarSelection()`, która czyści zaznaczenie:

```cpp
void Application::mouseButtonCallback(GLFWwindow* window, int button, int action, int /*mods*/) {
  auto* h = static_cast<WindowHooks*>(glfwGetWindowUserPointer(window));
  if (!h || !h->app) {
    return;
  }

  // Prawy przycisk — anuluj wybór auta i zakończ.
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    h->app->consumeEscapeForCarSelection();
    return;
  }

  if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS) {
    return;
  }
  // ... dalej bez zmian ...
```

**Uwaga:** `consumeEscapeForCarSelection()` to publiczna metoda `Application` (zadeklarowana
w `Application.hpp`), więc nie trzeba nic dodawać do nagłówka.

---

## Zadanie 2.3 — Pokazanie liczby zaparkowanych aut w tytule okna (zmiana sposobu wyświetlania)

**Treść polecenia (od prowadzącego):**
> Gdy panel ustawień jest otwarty, tytuł okna pokazuje wymiary i liczbę miejsc. Rozszerz tę
> informację o **liczbę aktualnie zaparkowanych aut** w formacie np. `auta: 8`. Policz tylko
> obiekty będące samochodami (`PropKind::Car`), nie lampy.

**Pliki do modyfikacji:**
- `src/app/Application.cpp` — funkcja `updateWindowTitle()`.

**Rozwiązanie:**

Oryginał:

```146:160:src/app/Application.cpp
void Application::updateWindowTitle() {
  if (!settingsOpen_) {
    glfwSetWindowTitle(window_.native(), kTitleNormal);
    return;
  }
  char buf[200];
  const char* lum =
      scene_.lighting().mode() == LightingMode::Day ? "dzien" : "noc";
  std::snprintf(buf, sizeof(buf),
                "USTAWIENIA [%d rzedow x %d miejsc = %d | %s]  [ ] miejsca, , . rzedy, N swiatlo, ESC  (%.1f x %.1f m)",
                uiRowCount_, uiSpotsPerRow_, scene_.generator().spotCount(), lum,
                static_cast<double>(scene_.generator().length()),
                static_cast<double>(scene_.generator().width()));
  glfwSetWindowTitle(window_.native(), buf);
}
```

Po zmianie — liczymy auta i dokładamy je do formatu (zwiększamy też bufor):

```cpp
void Application::updateWindowTitle() {
  if (!settingsOpen_) {
    glfwSetWindowTitle(window_.native(), kTitleNormal);
    return;
  }
  int carCount = 0;
  for (const auto& p : scene_.props()) {
    if (p.kind == PropKind::Car) {
      ++carCount;
    }
  }
  char buf[240];
  const char* lum =
      scene_.lighting().mode() == LightingMode::Day ? "dzien" : "noc";
  std::snprintf(buf, sizeof(buf),
                "USTAWIENIA [%d rzedow x %d miejsc = %d | %s | auta: %d]  [ ] miejsca, , . rzedy, N swiatlo, ESC  (%.1f x %.1f m)",
                uiRowCount_, uiSpotsPerRow_, scene_.generator().spotCount(), lum, carCount,
                static_cast<double>(scene_.generator().length()),
                static_cast<double>(scene_.generator().width()));
  glfwSetWindowTitle(window_.native(), buf);
}
```

`PropKind` i `scene_.props()` są dostępne przez dołączony już `scene/ParkingScene.hpp`.

**Pytanie kontrolne:** kiedy ten tytuł się odświeży? — Przy każdym wywołaniu
`updateWindowTitle()`, czyli m.in. po otwarciu panelu i po zmianach `[`, `]`, `,`, `.`, `N`.

---

# Kryteria oceny

| Aspekt | Punkty |
|--------|:-----:|
| Zmiana w **poprawnym pliku i funkcji** | 40% |
| **Poprawność logiczna** (zachowanie zgodne z poleceniem) | 40% |
| Brak skutków ubocznych / kompiluje się / spójne komentarze | 20% |

> **Wskazówka końcowa:** zanim zaczniesz pisać, prześledź ścieżkę zdarzenia:
> *wejście (GLFW callback / `glfwGetKey`)* → *`Application`* → *`ParkingScene` / `Lighting`*.
> Zrozumienie tego przepływu jest tu ważniejsze niż znajomość OpenGL-a.
