#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 uModel;
uniform mat4 uViewProj;
uniform mat4 uLightViewProj;

out vec4 vFragPosLightSpace;
out vec2 vGroundUv;
out vec3 vWorldPos;

void main() {
  vec4 wp = uModel * vec4(aPos, 1.0);
  vWorldPos = wp.xyz;
  vFragPosLightSpace = uLightViewProj * wp;
  vGroundUv = wp.xz;
  gl_Position = uViewProj * wp;
}
