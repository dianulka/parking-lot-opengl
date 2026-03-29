# Uruchomienie projektu (dla deweloperów)

## Wymagania

- **CMake** 3.20 lub nowszy  
- **Kompilator C++17** (MSVC 2019+, GCC 10+, Clang 12+)  
- **Git**  
- **Połączenie z internetem** przy **pierwszej** konfiguracji CMake (pobranie GLFW i GLM)

Zależności **GLFW** i **GLM** są ściągane przez `FetchContent`. **GLAD** (OpenGL 3.3 Core) leży w `third_party/glad/`. **Assimp nie jest używany** (brak zewnętrznych modeli aut).

## Szybki start

```bash
cmake -B build -S .
cmake --build build --config Release
```

### Windows (Visual Studio)

```powershell
cmake -B build -S . -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

- **Windows:** `build/Release/ParkingLot.exe`  
- **Linux (Ninja):** `./build/ParkingLot`

Przykład:

```bash
./build/ParkingLot --spaces 24 --length 72
```

```powershell
.\build\Release\ParkingLot.exe -n 24 -l 72
```

Ścieżka do `assets/` jest wpisana w binarkę jako `PARKING_ASSETS_DIR` (ścieżka bezwzględna z konfiguracji CMake).

## Linux

Ewentualnie pakiety pod GLFW/X11, np. Debian/Ubuntu:

```bash
sudo apt-get install -y libgl1-mesa-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev
```

## Regeneracja GLAD (opcjonalnie)

```bash
pip install glad2
python -m glad --api gl:core=3.3 --out-path third_party/glad --reproducible c
```

## Układ katalogów (skrót)

| Obszar   | Ścieżka           |
|----------|-------------------|
| OpenGL   | `src/gl/`         |
| Render   | `src/rendering/`  |
| Parking  | `src/scene/`      |
| Drzewa   | `src/models/TreeMesh.cpp` (proceduralnie) |
