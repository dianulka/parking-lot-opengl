#include "app/Application.hpp"
#include "scene/ParkingGenerator.hpp"
#include "scene/ParkingScene.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>

namespace {

void printUsage() {
  std::fprintf(stderr,
               "Uzycie: ParkingLot [opcje]\n"
               "  --spaces N, -n N   liczba miejsc parkingowych (domyslnie wg generatora)\n"
               "  --length L, -l L   dlugosc parkingu [m] (domyslnie wg generatora)\n"
               "  -h, --help         ta pomoc\n");
}

bool parseInt(const char* s, int& out) {
  char* end = nullptr;
  const long v = std::strtol(s, &end, 10);
  if (end == s || *end != '\0') {
    return false;
  }
  out = static_cast<int>(v);
  return true;
}

bool parseFloat(const char* s, float& out) {
  char* end = nullptr;
  const float v = std::strtof(s, &end);
  if (end == s || *end != '\0') {
    return false;
  }
  out = v;
  return true;
}

}  // namespace

int main(int argc, char** argv) {
  parking::ParkingGenerator gen;

  for (int i = 1; i < argc; ++i) {
    const char* a = argv[i];
    if (std::strcmp(a, "-h") == 0 || std::strcmp(a, "--help") == 0) {
      printUsage();
      return 0;
    }
    if ((std::strcmp(a, "--spaces") == 0 || std::strcmp(a, "-n") == 0)) {
      if (i + 1 >= argc) {
        std::fprintf(stderr, "Brak wartosci dla %s\n", a);
        return 1;
      }
      int n = 0;
      if (!parseInt(argv[++i], n)) {
        std::fprintf(stderr, "Nieprawidlowa liczba miejsc: %s\n", argv[i]);
        return 1;
      }
      gen.setSpotCount(n);
      continue;
    }
    if ((std::strcmp(a, "--length") == 0 || std::strcmp(a, "-l") == 0)) {
      if (i + 1 >= argc) {
        std::fprintf(stderr, "Brak wartosci dla %s\n", a);
        return 1;
      }
      float L = 0.0f;
      if (!parseFloat(argv[++i], L)) {
        std::fprintf(stderr, "Nieprawidlowa dlugosc: %s\n", argv[i]);
        return 1;
      }
      gen.setLength(L);
      continue;
    }
    std::fprintf(stderr, "Nieznany argument: %s\n", a);
    printUsage();
    return 1;
  }

  try {
    parking::ParkingScene scene(std::move(gen));
    parking::Application app(std::move(scene));
    return app.run();
  } catch (const std::exception& e) {
    std::fprintf(stderr, "Fatal: %s\n", e.what());
    return 1;
  }
}
