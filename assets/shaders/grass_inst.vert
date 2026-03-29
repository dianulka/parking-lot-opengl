#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 3) in vec4 aInst; // x, z, rotY, scale

uniform mat4 uViewProj;
uniform mat4 uLightViewProj;
uniform float uTime;
uniform float uWindFreq;
uniform float uWindAmp;

const float kBladeH = 1.25;

out vec3 vNormal;
out vec4 vFragPosLightSpace;
out float vTip;

void main() {
  vec3 base = vec3(aPos.x * aInst.w, aPos.y * aInst.w, aPos.z * aInst.w);

  float h = aPos.y / kBladeH;
  float bend = h * h;
  float ph = uTime * uWindFreq + aInst.x * 0.41 + aInst.y * 0.37;
  float ph2 = uTime * uWindFreq * 1.55 + aInst.x * 0.11 - aInst.y * 0.09;

  float swayX = sin(ph) * uWindAmp * bend + sin(ph2) * uWindAmp * 0.28 * bend;
  float swayZ = sin(ph * 0.73 + 1.9) * uWindAmp * 0.62 * bend;
  base.x += swayX;
  base.z += swayZ;

  float c = cos(aInst.z);
  float s = sin(aInst.z);
  vec3 rot = vec3(c * base.x - s * base.z, base.y, s * base.x + c * base.z);
  vec3 world = rot + vec3(aInst.x, 0.05625, aInst.y);

  vec3 nloc = vec3(c * aNormal.x - s * aNormal.z, aNormal.y, s * aNormal.x + c * aNormal.z);
  vNormal = normalize(nloc);

  vTip = clamp(h, 0.0, 1.0);
  gl_Position = uViewProj * vec4(world, 1.0);
  vFragPosLightSpace = uLightViewProj * vec4(world, 1.0);
}
