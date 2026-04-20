#version 330 core
in vec3 vNormal;
in vec4 vFragPosLightSpace;
in float vTip;
in vec3 vWorldPos;

#define MAX_POINT_LIGHTS 96

uniform vec3 uLightDir;
uniform float uAmbient;
uniform sampler2D uShadowMap;

uniform float uDirectionalWeight;
uniform vec3 uSunColor;
uniform float uSunDiskWeight;
uniform vec3 uSunDiskWorldPos;
uniform float uSunDiskIntensity;
uniform float uSunDiskRadius;
uniform vec3 uSunDiskColor;

uniform int uNumPointLights;
uniform vec3 uPointPos[MAX_POINT_LIGHTS];
uniform vec3 uPointColor;
uniform float uPointIntensity;
uniform float uPointRadius;
uniform float uGrassPointLightScale;

out vec4 FragColor;

float shadowFactor(vec4 fragPosLightSpace, vec3 n) {
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  projCoords = projCoords * 0.5 + 0.5;
  if (projCoords.z > 1.0) {
    return 1.0;
  }
  vec2 texel = 1.0 / textureSize(uShadowMap, 0);
  vec3 L = normalize(uLightDir);
  float bias = max(0.0015 * (1.0 - dot(n, L)), 0.00035);
  float cur = projCoords.z;
  float s = 0.0;
  for (int x = -1; x <= 1; ++x) {
    for (int y = -1; y <= 1; ++y) {
      float d = texture(uShadowMap, projCoords.xy + vec2(float(x), float(y)) * texel).r;
      s += (cur - bias > d) ? 0.35 : 1.0;
    }
  }
  return s / 9.0;
}

void main() {
  vec3 n = normalize(vNormal);
  vec3 L = normalize(uLightDir);
  vec3 baseDark = vec3(0.12, 0.38, 0.1);
  vec3 baseLight = vec3(0.22, 0.62, 0.18);
  vec3 albedo = mix(baseDark, baseLight, vTip);

  float nd = max(dot(n, L), 0.0);
  float sh = shadowFactor(vFragPosLightSpace, n);
  float litDir = uAmbient + (1.0 - uAmbient) * nd * sh;

  vec3 sunDisk = vec3(0.0);
  if (uSunDiskWeight > 0.001) {
    vec3 toD = uSunDiskWorldPos - vWorldPos;
    float dD = length(toD);
    vec3 Ld = toD / max(dD, 1e-3);
    float ndd = max(dot(n, Ld), 0.0);
    float rD2 = max(uSunDiskRadius * uSunDiskRadius, 60000.0);
    float attD = uSunDiskIntensity / (1.0 + 2.2e-5 * dD + (dD * dD) / rD2);
    sunDisk = albedo * uSunDiskColor * ndd * attD * uSunDiskWeight * 0.11;
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
  pointDiff *= uGrassPointLightScale;

  vec3 lit =
      albedo * litDir * uSunColor * uDirectionalWeight + sunDisk + albedo * pointDiff;
  FragColor = vec4(lit, 1.0);
}
