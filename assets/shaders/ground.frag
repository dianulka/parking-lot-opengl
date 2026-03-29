#version 330 core
in vec4 vFragPosLightSpace;
in vec2 vGroundUv;
in vec3 vWorldPos;

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
  float lit = uAmbient + (1.0 - uAmbient) * nd * sh;

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

  FragColor = vec4(base * lit, 1.0);
}
