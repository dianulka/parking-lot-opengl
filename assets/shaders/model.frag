#version 330 core
in vec3 vNormal;
in vec3 vWorldPos;
in vec4 vFragPosLightSpace;
in vec2 vTex;

uniform vec3 uBaseColor;
uniform vec3 uLightDir;
uniform vec3 uCameraPos;
uniform float uAmbient;
uniform float uSpecularStrength;
uniform sampler2D uShadowMap;
uniform sampler2D uDiffuse;
uniform bool uUseTexture;

out vec4 FragColor;

float shadowFactor(vec4 fragPosLightSpace, vec3 n) {
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  projCoords = projCoords * 0.5 + 0.5;
  if (projCoords.z > 1.0) {
    return 1.0;
  }
  vec2 texel = 1.0 / textureSize(uShadowMap, 0);
  vec3 L = normalize(uLightDir);
  float bias = max(0.002 * (1.0 - dot(n, L)), 0.0004);
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
  vec3 V = normalize(uCameraPos - vWorldPos);
  vec3 H = normalize(L + V);

  vec3 albedo = uUseTexture ? texture(uDiffuse, vTex).rgb * uBaseColor : uBaseColor;

  float nd = max(dot(n, L), 0.0);
  float sh = shadowFactor(vFragPosLightSpace, n);
  float dirLight = uAmbient + (1.0 - uAmbient) * nd * sh;

  float specMask = uUseTexture ? 0.55 : 0.35;
  float spec = pow(max(dot(n, H), 0.0), 40.0) * uSpecularStrength * specMask;

  FragColor = vec4(albedo * dirLight + vec3(spec), 1.0);
}
