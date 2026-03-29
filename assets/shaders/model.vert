#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTex;

uniform mat4 uModel;
uniform mat4 uViewProj;
uniform mat4 uLightViewProj;
uniform mat3 uNormalMat;

out vec3 vNormal;
out vec3 vWorldPos;
out vec4 vFragPosLightSpace;
out vec2 vTex;

void main() {
  vec4 wp = uModel * vec4(aPos, 1.0);
  vWorldPos = wp.xyz;
  vNormal = uNormalMat * aNormal;
  vTex = aTex;
  gl_Position = uViewProj * wp;
  vFragPosLightSpace = uLightViewProj * wp;
}
