#version 330 core
layout (location = 0) in vec2 aPos;

uniform mat4 uInvViewProj;

out vec3 vViewDir;

void main() {
  // Z = 0.0 (near) and Z = 1.0 (far) in NDC; unproject both to recover the world-space view ray.
  vec4 nearH = uInvViewProj * vec4(aPos, -1.0, 1.0);
  vec4 farH  = uInvViewProj * vec4(aPos,  1.0, 1.0);
  vec3 pNear = nearH.xyz / nearH.w;
  vec3 pFar  = farH.xyz  / farH.w;
  vViewDir = pFar - pNear;

  // Wypchnięcie kwadratu na płaszczyznę dalszą; rysujemy z wyłączonym testem głębi.
  gl_Position = vec4(aPos, 1.0, 1.0);
}
