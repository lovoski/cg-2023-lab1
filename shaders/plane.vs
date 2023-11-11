#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 Normal;
out vec3 FragPos;
out vec4 FragPosLightSpace;
uniform mat4 lightSpaceMatrix;

uniform mat4 view;
uniform mat4 projection;

void main() {
  FragPosLightSpace = lightSpaceMatrix*vec4(aPos, 1.0);

  gl_Position = projection*view*vec4(aPos, 1.0);
  FragPos = vec3(gl_Position);

  Normal = aNormal;
}