#version 330 core
in vec3 vNormal;
in vec4 vFragPosLightSpace;
in float vTip;

uniform vec3 uLightDir;
uniform float uAmbient;
uniform sampler2D uShadowMap;

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
  float lit = uAmbient + (1.0 - uAmbient) * nd * sh;

  FragColor = vec4(albedo * lit, 1.0);
}
