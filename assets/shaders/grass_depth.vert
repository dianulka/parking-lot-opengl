#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 3) in vec4 aInst;

uniform mat4 uLightViewProj;
uniform float uTime;
uniform float uWindFreq;
uniform float uWindAmp;

const float kBladeH = 30.0;

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
  gl_Position = uLightViewProj * vec4(world, 1.0);
}
