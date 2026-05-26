#version 330 core
in vec3 vViewDir;

uniform int uIsNight;       // 0 = dzień, 1 = noc
uniform vec3 uSunDir;       // kierunek do słońca / księżyca (znormalizowany)
uniform float uTime;        // czas w sekundach (dryf chmur, migotanie gwiazd)

out vec4 FragColor;

float hash13(vec3 p) {
  p = fract(p * vec3(443.897, 441.423, 437.195));
  p += dot(p, p.yzx + 19.19);
  return fract((p.x + p.y) * p.z);
}

float hash12(vec2 p) {
  vec3 p3 = fract(vec3(p.xyx) * 0.1031);
  p3 += dot(p3, p3.yzx + 33.33);
  return fract((p3.x + p3.y) * p3.z);
}

float valueNoise(vec2 p) {
  vec2 i = floor(p);
  vec2 f = fract(p);
  vec2 u = f * f * (3.0 - 2.0 * f);
  float a = hash12(i);
  float b = hash12(i + vec2(1.0, 0.0));
  float c = hash12(i + vec2(0.0, 1.0));
  float d = hash12(i + vec2(1.0, 1.0));
  return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

float fbm(vec2 p) {
  float v = 0.0;
  float a = 0.5;
  for (int i = 0; i < 5; ++i) {
    v += a * valueNoise(p);
    p *= 2.07;
    a *= 0.5;
  }
  return v;
}

// Pokrycie chmur w danym kierunku patrzenia (0 = czyste niebo, 1 = pełna chmura).
float cloudCoverage(vec3 d, float timeSec) {
  if (d.y < 0.04) {
    return 0.0;
  }
  // Rzut promienia na "kopułę" chmur — im niżej patrzymy, tym bardziej UV się rozciąga (perspektywa horyzontu).
  vec2 uv = d.xz / max(d.y, 0.06);
  uv *= 0.45;
  vec2 drift = vec2(timeSec * 0.0040, timeSec * 0.0026);
  vec2 uv1 = uv + drift;
  vec2 uv2 = uv * 2.7 + drift * 2.1 + vec2(13.7, -5.1);

  float baseShape = fbm(uv1);
  // „Kępkowanie” chmur — wyciągnięcie ostrzejszych kształtów z FBM.
  float cumulus = smoothstep(0.46, 0.82, baseShape);
  // Drobny detal wewnątrz chmur — falowanie kraw.
  float detail = fbm(uv2);
  float shaped = clamp(cumulus * (0.55 + 0.55 * detail), 0.0, 1.0);

  // Wygaszenie chmur tuż nad horyzontem (mgła) i bardzo wysoko w zenicie (mniej chmur prosto nad głową).
  float horizonFade = smoothstep(0.06, 0.28, d.y);
  float zenithFade  = 1.0 - smoothstep(0.85, 1.0, d.y) * 0.25;
  return clamp(shaped * horizonFade * zenithFade, 0.0, 1.0);
}

vec3 daySky(vec3 d, vec3 sunDir, float timeSec) {
  float h = clamp(d.y, -1.0, 1.0);

  // Mocniejszy, czystszy błękit; horyzont jaśniejszy, zenit nasycony.
  vec3 zenith   = vec3(0.16, 0.42, 0.86);
  vec3 horizon  = vec3(0.82, 0.90, 0.99);
  vec3 below    = vec3(0.60, 0.58, 0.52);
  float t = smoothstep(-0.02, 0.55, h);
  vec3 sky = mix(horizon, zenith, t);
  if (h < 0.0) {
    sky = mix(horizon, below, clamp(-h * 1.6, 0.0, 1.0));
  }

  // Ciepły pas przy horyzoncie w kierunku słońca.
  float horizonGlow = exp(-abs(h) * 5.5) * 0.32;
  vec3 warm = vec3(1.00, 0.78, 0.55);
  float sunAlong = max(dot(normalize(vec3(d.x, 0.0, d.z)), normalize(vec3(sunDir.x, 0.0, sunDir.z))), 0.0);
  sky += warm * horizonGlow * pow(sunAlong, 6.0) * 0.55;

  // Dysk słońca + halo.
  float cs = max(dot(normalize(d), normalize(sunDir)), 0.0);
  float halo = pow(cs, 28.0) * 0.30 + pow(cs, 240.0) * 1.35;
  float disk = smoothstep(0.9988, 0.99975, cs);
  vec3 sunCol = vec3(1.00, 0.96, 0.86);
  sky += sunCol * (halo + disk * 3.2);

  // Chmury — biel z lekkim cieniem od spodu i rozjaśnieniem w stronę słońca.
  float cov = cloudCoverage(d, timeSec);
  if (cov > 0.0) {
    float sunFacing = max(dot(normalize(d), normalize(sunDir)), 0.0);
    vec3 cloudLit   = vec3(1.00, 0.99, 0.96);
    vec3 cloudShade = vec3(0.62, 0.66, 0.74);
    // Bliżej słońca chmury jaśniejsze; w zenicie ciemniej u spodu.
    vec3 cloudCol = mix(cloudShade, cloudLit, pow(sunFacing, 1.8) * 0.85 + 0.15);
    // Lekki srebrzysty rant przy słońcu („silver lining”).
    float lining = pow(sunFacing, 14.0) * 0.55;
    cloudCol += vec3(1.0, 0.95, 0.82) * lining * cov;
    sky = mix(sky, cloudCol, cov);
  }

  return sky;
}

vec3 nightSky(vec3 d, vec3 moonDir, float timeSec) {
  float h = clamp(d.y, -1.0, 1.0);

  // Ciemny gradient: granat → prawie czerń w zenicie; poniżej horyzontu jeszcze ciemniej.
  vec3 zenith   = vec3(0.003, 0.006, 0.018);
  vec3 horizon  = vec3(0.028, 0.044, 0.100);
  vec3 below    = vec3(0.008, 0.010, 0.018);
  float t = smoothstep(0.0, 0.60, h);
  vec3 sky = mix(horizon, zenith, t);
  if (h < 0.0) {
    sky = mix(horizon, below, clamp(-h * 2.0, 0.0, 1.0));
  }

  // Gwiazdy — proceduralna siatka, próg na hash; tylko nad horyzontem.
  if (h > 0.02) {
    vec3 dn = normalize(d);
    // Dwie skale: drobne i większe.
    for (int layer = 0; layer < 2; ++layer) {
      float scale = (layer == 0) ? 220.0 : 95.0;
      float threshold = (layer == 0) ? 0.9968 : 0.9988;
      float boost = (layer == 0) ? 1.0 : 1.8;
      vec3 q = dn * scale;
      vec3 qi = floor(q);
      float r = hash13(qi);
      if (r > threshold) {
        float r2 = hash13(qi + 17.31);
        float twinkle = 0.55 + 0.45 * sin(timeSec * (1.4 + r2 * 2.6) + r2 * 31.0);
        float brightness = (r - threshold) / (1.0 - threshold);
        vec3 tint = mix(vec3(0.85, 0.92, 1.00), vec3(1.00, 0.92, 0.78), r2);
        sky += tint * brightness * twinkle * boost * smoothstep(0.02, 0.18, h);
      }
    }
  }

  // Księżyc — dysk z miękkim halo.
  vec3 mDir = normalize(moonDir);
  float cm = max(dot(normalize(d), mDir), 0.0);
  float disk  = smoothstep(0.9982, 0.9995, cm);
  float halo  = pow(cm, 90.0) * 0.10 + pow(cm, 800.0) * 0.5;
  vec3 moonCol = vec3(0.86, 0.88, 0.95);
  sky += moonCol * (halo + disk * 1.3);

  return sky;
}

void main() {
  vec3 d = normalize(vViewDir);
  vec3 c = (uIsNight == 1) ? nightSky(d, uSunDir, uTime) : daySky(d, uSunDir, uTime);
  FragColor = vec4(c, 1.0);
}
