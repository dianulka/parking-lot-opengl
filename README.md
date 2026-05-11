# ```Symulacja parkingu prostokątnego w OpenGL z parametryzowaną liczbą miejsc postojowych oraz parametryzowanym wymiarem zewnętrznym```

**Bieżąca implementacja (kod):** modele glTF z `assets/models/` (AE86 na miejscach, lampy sci‑fi przy liniach parkingu i przy pasie drogi); **jedna duża plansza trawy**; asfalt tylko na **parking L×W** i na **pasie drogi** przez trawę (bez dodatkowego asfaltu poza obrysem parkingu); światło kierunkowe i mapa cieni; parametry `--spaces` / `--length`.

### Jak uruchomić (GitHub / nowy komputer)

1. Zainstaluj **CMake 3.20+** i **kompilator C++17** (np. Visual Studio 2022 z „Desktop development with C++” na Windowsie).
2. W katalogu repozytorium:
   ```bash
   cmake -B build -S .
   cmake --build build --config Release
   ```
   Pierwsza konfiguracja wymaga **internetu** (pobierane są GLFW i GLM przez CMake).
3. Uruchom:
   - Windows: `build\Release\ParkingLot.exe`
   - Linux (Ninja/Make): zwykle `./build/ParkingLot`

Przykład: `ParkingLot.exe -n 32 -l 80` — szczegóły w [docs/SETUP.md](docs/SETUP.md).

**GitHub:** nie commituj katalogu `build/` (jest w `.gitignore`). W repozytorium: `CMakeLists.txt`, `src/`, `assets/shaders/`, `third_party/glad/`, `third_party/stb/`. CI: [`.github/workflows/build.yml`](.github/workflows/build.yml).

---

## 1. Cel rozwiązania
Celem rozwiązania jest opracowanie aplikacji 3D w technologii OpenGL, która umożliwia wizualizację parkingu o planie prostokąta.
Podstawowym celem funkcjonalnym projektu jest umożliwienie określenia:
- liczby miejsc parkingowych,
- długości parkingu jako wybranego parametru zewnętrznego.

Na podstawie tych dwóch parametrów program tworzy scenę trójwymiarową obejmującą:
- nawierzchnię parkingu,
- dwa rzędy miejsc postojowych,
- pas przejazdu pomiędzy rzędami,
- linie wyznaczające miejsca postojowe,
- samochody zaparkowane na wybranych miejscach,
- lampy oświetleniowe,
- drzewa jako elementy otoczenia.

## 2. Adresaci rozwiązania
Adresatami rozwiązania są osoby oraz podmioty zajmujące się planowaniem i organizacją przestrzeni parkingowej. Aplikacja może być wykorzystywana przez projektantów zagospodarowania terenu, zarządców nieruchomości oraz firmy planujące infrastrukturę parkingową przy budynkach mieszkalnych, usługowych lub handlowych.

Rozwiązanie może służyć do szybkiej wizualizacji układu parkingu, rozmieszczenia miejsc postojowych oraz elementów infrastruktury takich jak lampy czy zieleń, co ułatwia analizę różnych wariantów organizacji przestrzeni parkingowej.
## 3. Funkcje i możliwości rozwiązania
Rozwiązanie będzie umożliwiało wygenerowanie parkingu naziemnego o kształcie prostokąta, zorganizowanego w postaci dwóch rzędów miejsc parkingowych rozdzielonych centralnym pasem przejazdu. Układ ten będzie tworzony automatycznie na podstawie dwóch parametrów wejściowych.

## Podstawowe funkcje:
1. Generowanie parkingu \
Program tworzy prostokątną powierzchnię parkingu o zadanej długości. Szerokość pozostaje stała, ponieważ układ dwóch rzędów miejsc i środkowego pasa przejazdu jest z góry określony.
2. Parametryzacja liczby miejsc parkingowych \
Użytkownik określa liczbę miejsc parkingowych. Program automatycznie rozmieszcza miejsca w dwóch równoległych rzędach. Liczba stanowisk wpływa na ostateczny wygląd i zagęszczenie układu.
3. Parametryzacja długości parkingu \
Użytkownik określa długość parkingu jako zmienny parametr zewnętrzny. Po zmianie tej wartości geometria parkingu jest przeliczana i dostosowywana do nowego rozmiaru.
4. Rysowanie linii parkingowych \
Dla każdego miejsca parkingowego program generuje linie wyznaczające granice stanowiska. Linie są rozmieszczone zgodnie z układem dwóch rzędów i tworzą czytelne oznaczenie miejsc postojowych.
5. Umieszczanie samochodów \
Na części stanowisk program umieszcza samochody. Samochody pochodzą z gotowych modeli 3D. Ich rozmieszczenie jest losowe.
6. Umieszczanie lamp \
Lampy są rozmieszczane przy krawędziach parkingu w logicznych odstępach. W trybie nocnym lampy pełnią rolę aktywnych źródeł światła.
7. Umieszczanie drzew
Drzewa są rozmieszczane w otoczeniu parkingu jako elementy zieleni poprawiające czytelność i estetykę sceny.
9. Tryb dzień / noc \
Program umożliwia przełączanie pomiędzy dwoma stanami wizualizacji:
- tryb dzienny: jasna scena, dominujące światło ogólne,
- tryb nocny: ciemniejsza scena, aktywne lampy jako źródła światła.

## 4. Rzeczywistość modelowana w rozwiązaniu (case study)
Rozwiązanie modeluje uproszczony, zewnętrzny parking naziemny o regularnym, prostokątnym planie. Parking jest zorganizowany w sposób typowy dla niewielkich obiektów usługowych, osiedlowych lub użyteczności publicznej: dwa równoległe rzędy miejsc parkingowych i droga manewrowa pośrodku.

Modelowana rzeczywistość obejmuje:
- utwardzoną nawierzchnię parkingową,
- stanowiska postojowe,
- oznaczenia poziome miejsc,
- zaparkowane pojazdy,
- lampy parkingowe,
- elementy zieleni,
- zmienne warunki oświetleniowe odpowiadające porze dnia.

Jest to model statyczny. Program nie odwzorowuje ruchu pojazdów, manewrów parkowania, pieszych, kolizji ani szczegółowych zasad ruchu drogowego. Nie uwzględnia też norm projektowych ani rzeczywistych wymagań budowlanych. Celem modelu jest pokazanie logicznej i wizualnej struktury parkingu.

## 5. Obiekty przetwarzane przez oferowane funkcje rozwiązania, obiekty powiązane

### Obiekty główne

#### Parking
Centralny obiekt sceny, reprezentujący cały obszar przeznaczony na postój i manewrowanie pojazdami. Zawiera:
- obrys prostokątny,
- długość,
- szerokość,
- nawierzchnię,
- logiczny układ miejsc i pasa przejazdu.

#### Miejsce parkingowe
Obiekt reprezentujący pojedyncze stanowisko postojowe. Każde miejsce posiada:
- pozycję na scenie,
- orientację,
- wymiary,
- przypisanie do lewego lub prawego rzędu,
- status: wolne lub zajęte.

#### Pas przejazdu
Obiekt logiczny i geometryczny znajdujący się pomiędzy dwoma rzędami miejsc. Jego zadaniem jest rozdzielenie stanowisk i nadanie parkingowi realistycznego układu przestrzennego.

#### Linie parkingowe
Obiekty wizualne wyznaczające granice stanowisk. Są generowane automatycznie na podstawie położenia miejsc parkingowych.

#### Samochód
Obiekt reprezentujący zaparkowany pojazd. Każdy samochód jest powiązany z jednym miejscem parkingowym. Posiada:
- model 3D,
- pozycję,
- skalę,
- orientację zgodną z miejscem postojowym.

### Obiekty powiązane

#### Lampa
Obiekt infrastrukturalny umieszczony przy krawędziach parkingu. W sensie wizualnym reprezentuje słup oświetleniowy, a w sensie funkcjonalnym może działać jako źródło światła w trybie nocnym.

#### Drzewo
Obiekt dekoracyjny umieszczony poza głównym obszarem parkowania. Ma wpływ głównie na wygląd sceny i organizację przestrzeni wizualnej.

#### Kamera
Obiekt techniczny definiujący sposób obserwacji sceny. Ustawiona jest tak, aby użytkownik widział cały parking pod lekkim kątem z góry.

#### Źródła światła
Obiekty logiczne odpowiedzialne za oświetlenie sceny:
- światło ogólne dzienne,
- światło ogólne nocne,
- światła lokalne przypisane lampom.

### Relacje między obiektami
- parking zawiera miejsca parkingowe i pas przejazdu,
- miejsca są rozmieszczone w dwóch rzędach,
- linie parkingowe są związane z granicami miejsc,
- samochód może być przypisany do konkretnego miejsca,
- lampy i drzewa są rozmieszczone względem zewnętrznych granic parkingu,
- tryb dzień/noc wpływa na światła i wygląd całej sceny,
- kamera obserwuje wszystkie obiekty w jednej wspólnej przestrzeni.

## 6. Ramowe wymagania funkcjonalne stawiane rozwiązaniu

1. Program musi generować parking o planie prostokąta.
2. Program musi umożliwiać ustawienie liczby miejsc parkingowych.
3. Program musi umożliwiać ustawienie długości parkingu jako zmiennego parametru zewnętrznego.
4. Program musi rozmieszczać miejsca parkingowe automatycznie w dwóch rzędach.
5. Program musi tworzyć pas przejazdu pomiędzy rzędami miejsc.
6. Program musi automatycznie rysować linie wyznaczające miejsca parkingowe.
7. Program musi wyświetlać samochody na wybranych stanowiskach.
8. Program musi wyświetlać drzewa jako elementy otoczenia.
9. Program musi wyświetlać lampy jako elementy infrastruktury parkingu.
10. Program musi umożliwiać przełączanie między trybem dziennym i nocnym.
11. Program musi zmieniać warunki oświetleniowe po przełączeniu trybu.
12. Program musi wyświetlać scenę w widoku 3D.
13. Program musi umożliwiać sterowanie podstawowymi funkcjami za pomocą klawiatury.
14. Program musi zachować spójność geometrii po zmianie parametrów.
15. Program powinien uniemożliwiać powstawanie miejsc nachodzących na siebie lub wychodzących poza powierzchnię parkingu.
16. Program powinien rozmieszczać lampy i drzewa w stałych, logicznych pozycjach względem granic parkingu.
17. Program powinien umożliwiać wielokrotne przełączanie trybu dzień/noc bez błędów działania.
18. Program powinien umożliwiać ponowne wygenerowanie układu po zmianie liczby miejsc lub długości parkingu.

## 7. Wstępny opis interfejsu użytkownika

Interfejs użytkownika będzie miał postać pojedynczego okna aplikacji z widokiem sceny 3D. Komunikacja użytkownika z programem będzie realizowana za pomocą klawiatury.

### Elementy interfejsu
- główne okno renderowania,
- widok parkingu w przestrzeni 3D,
- ewentualne informacje tekstowe wypisywane w konsoli.

### Sposób działania
Po uruchomieniu programu użytkownik widzi kompletną scenę parkingu wygenerowaną na podstawie wartości początkowych. Następnie może z poziomu klawiatury zmieniać podstawowe parametry lub tryb pracy.

### Proponowane sterowanie
- `N` – przełączenie trybu dzień/noc,
- `+` – zwiększenie liczby miejsc parkingowych,
- `-` – zmniejszenie liczby miejsc parkingowych,
- `L` – zwiększenie długości parkingu,
- `K` – zmniejszenie długości parkingu,
- `ESC` – zamknięcie programu.

Opcjonalnie:
- strzałki lub `WSAD` – zmiana położenia albo kąta kamery.

W bieżącej aplikacji dodatkowo:
- lewy przycisk myszy (poza przyciskiem ustawień w lewym górnym rogu) — klik na samochód, następnie klik na wolne miejsce w rzędzie (nie na środkowy pas); `ESC` przy wybranym aucie anuluje wybór zamiast zamykać okno;
- panel ustawień: `[` `]` — liczba miejsc, `N` — dzień / noc (szczegóły w konsoli przy starcie).

## 8. Wymagania niefunkcjonalne


### Czytelność kodu
Kod powinien być uporządkowany i podzielony na logiczne części:
- inicjalizacja OpenGL,
- wczytanie modeli,
- generowanie parkingu,
- renderowanie sceny,
- obsługa wejścia z klawiatury,
- obsługa oświetlenia.

### Stabilność
Program powinien działać poprawnie przy wielokrotnej zmianie liczby miejsc, długości parkingu oraz przy przełączaniu trybu dzień/noc. Nie powinien zawieszać się ani kończyć działania z błędem przy standardowym użyciu.

### Łatwość demonstracji
Projekt powinien umożliwiać szybkie pokazanie najważniejszych funkcji podczas prezentacji:
- zmiany liczby miejsc,
- zmiany długości parkingu,
- przełączenia dzień/noc,
- obecności modeli samochodów, drzew i lamp.

## 9. Środki techniczne, jakie będą w rozwiązaniu wykorzystane
### Język programowania
Rozwiązanie będzie realizowane w języku **C++**.

### Technologia graficzna
Do renderowania sceny zostanie wykorzystane **OpenGL**.


### Biblioteki pomocnicze
- **GLFW** – do utworzenia okna aplikacji, obsługi kontekstu OpenGL i klawiatury,
- **GLAD** – do ładowania funkcji OpenGL,
- **GLM** – do obliczeń matematycznych, macierzy transformacji i wektorów,
- **Assimp** – do wczytywania gotowych modeli 3D samochodów, drzew i lamp.


### Modele gotowe
Z gotowych modeli 3D zostaną wykorzystane:
- samochody,
- drzewa,
- lampy.

### Obsługa wejścia
Interakcja użytkownika będzie oparta na zdarzeniach klawiaturowych. Program będzie reagował na wciśnięcie określonych klawiszy odpowiedzialnych za zmianę parametrów i trybu pracy.

### Oświetlenie
Zostanie zastosowane:
- światło ogólne dla sceny,
- punktowe lub lokalne źródła światła przypisane lampom,
- osobne ustawienia parametrów światła dla dnia i nocy.

---

## Środowisko programistyczne (CMake)

Konfiguracja projektu, zależności (GLFW, GLM przez CMake `FetchContent`, GLAD 3.3 Core w `third_party/glad`) oraz kompilacja: **[docs/SETUP.md](docs/SETUP.md)**.
