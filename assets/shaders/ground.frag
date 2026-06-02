#version 330 core
in vec4 vFragPosLightSpace;
in vec2 vGroundUv;
in vec3 vWorldPos;

#define MAX_POINT_LIGHTS 96

uniform vec3 uGrassTint;
uniform vec3 uRoadStripTint;
uniform vec3 uParkingTint;
uniform float uAmbient;
uniform vec3 uLightDir;
uniform sampler2D uShadowMap;
uniform sampler2D uGrassAlbedo;
uniform sampler2D uRoadAlbedo;
uniform float uGrassTexScale;
uniform float uRoadTexScale;
uniform float uRoadTexScaleStrip;

uniform float uHalfParkingL;
uniform float uHalfParkingW;
uniform float uHalfRoadW;
uniform float uHalfGrassL;
uniform bool uPotholeVisible;

uniform int uNumPointLights;
uniform vec3 uPointPos[MAX_POINT_LIGHTS];
uniform vec3 uPointColor;
uniform float uPointIntensity;
uniform float uPointRadius;

uniform float uDirectionalWeight;
uniform vec3 uSunColor;
uniform float uSunDiskWeight;
uniform vec3 uSunDiskWorldPos;
uniform float uSunDiskIntensity;
uniform float uSunDiskRadius;
uniform vec3 uSunDiskColor;

out vec4 FragColor;

float shadowFactor(vec4 fragPosLightSpace) {
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  projCoords = projCoords * 0.5 + 0.5;
  if (projCoords.z > 1.0) {
    return 1.0;
  }
  vec2 texel = 1.0 / textureSize(uShadowMap, 0);
  float bias = 0.0015;
  float cur = projCoords.z;
  float s = 0.0;
  for (int ix = -1; ix <= 1; ++ix) {
    for (int iy = -1; iy <= 1; ++iy) {
      float d = texture(uShadowMap, projCoords.xy + vec2(float(ix), float(iy)) * texel).r;
      s += (cur - bias > d) ? 0.35 : 1.0;
    }
  }
  return s / 9.0;
}

void main() {
  vec3 n = vec3(0.0, 1.0, 0.0);
  vec3 L = normalize(uLightDir);
  float nd = max(dot(n, L), 0.0);
  float sh = shadowFactor(vFragPosLightSpace);
  float litDir = uAmbient + (1.0 - uAmbient) * nd * sh;

  float x = vWorldPos.x;
  float z = vWorldPos.z;
  vec2 uv = vGroundUv;

  // Zawsze próbkuj wszystkie albedo — inaczej część sterowników nie wiąże samplerów z jednostkami tekstur.
  vec3 colGrass = texture(uGrassAlbedo, uv * uGrassTexScale).rgb * uGrassTint;
  vec3 colPark = texture(uRoadAlbedo, uv * uRoadTexScale).rgb * uParkingTint;
  vec3 colRoad = texture(uRoadAlbedo, uv * uRoadTexScaleStrip).rgb * uRoadStripTint;

  vec3 base;
  if (abs(x) <= uHalfParkingL && abs(z) <= uHalfParkingW) {
    base = colPark;
  } else if (abs(x) <= uHalfGrassL && abs(z) <= uHalfRoadW) {
    base = colRoad;
  } else {
    base = colGrass;
  }
  if (uPotholeVisible && length((vec2(x, z) - vec2(0.0, 2.0)) / vec2(2.0, 1.0)) < 1.0) {
    base = vec3(0.015, 0.013, 0.012);
  }

  vec3 pointDiff = vec3(0.0);
  float r2 = max(uPointRadius * uPointRadius, 4.0);
  for (int i = 0; i < MAX_POINT_LIGHTS; ++i) {
    if (i >= uNumPointLights) {
      break;
    }
    vec3 toL = uPointPos[i] - vWorldPos;
    float dist = length(toL);
    vec3 Lp = toL / max(dist, 1e-4);
    float att = uPointIntensity / (1.0 + 0.065 * dist + 0.72 * (dist * dist) / r2);
    float rim = 1.0 - smoothstep(uPointRadius * 0.48, uPointRadius * 1.28, dist);
    att *= rim;
    float ndp = max(dot(n, Lp), 0.0);
    float spill = att * 0.048;
    pointDiff += uPointColor * (ndp * att * 1.92 + spill);
  }

  vec3 sunDisk = vec3(0.0);
  if (uSunDiskWeight > 0.001) {
    vec3 toD = uSunDiskWorldPos - vWorldPos;
    float dD = length(toD);
    vec3 Ld = toD / max(dD, 1e-3);
    float ndd = max(dot(n, Ld), 0.0);
    float rD2 = max(uSunDiskRadius * uSunDiskRadius, 60000.0);
    float attD = uSunDiskIntensity / (1.0 + 2.2e-5 * dD + (dD * dD) / rD2);
    sunDisk = base * uSunDiskColor * ndd * attD * uSunDiskWeight * 0.11;
  }

  vec3 outRgb = base * litDir * uSunColor * uDirectionalWeight + sunDisk + base * pointDiff;
  FragColor = vec4(outRgb, 1.0);
}
