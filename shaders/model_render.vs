#version 460 core
layout(location=0)in vec3 aPos;
// layout(location=1)in vec3 aNormal;
// layout(location=2)in vec2 aTexCoords;
layout(location=1)in vec2 aTexCoords;
// layout(location=3)in vec3 tangent;
// layout(location=4)in vec3 bitangent;

out vec2 TexCoords;
// out vec3 Normal;
// out vec3 FragPos;
// out mat3 TBN;

layout(std140,binding=0)uniform Object
{
  mat4 projection;
  mat4 view;
};

uniform mat4 model;
// uniform mat4 view;
// uniform mat4 projection;

void main()
{
  // pass texture coordinate to fragment shader
  TexCoords=aTexCoords;
  // make sure no point is clipped
  gl_Position=vec4(aPos,1.);
}