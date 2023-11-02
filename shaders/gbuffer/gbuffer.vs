#version 460 core
layout(location=0)in vec3 aPos;
layout(location=1)in vec3 aNormal;
layout(location=2)in vec2 aTexCoords;
layout(location=3)in vec3 tangent;
layout(location=4)in vec3 bitangent;

out vec2 TexCoords;
// out vec3 Normal;
out vec3 FragPos;
out mat3 TBN;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec3 offsets[2];

void main()
{
  TexCoords=aTexCoords;
  // Normal=aNormal;
  FragPos=vec3(model*vec4(aPos,1.));

  gl_Position=vec4(aPos,1.);
  gl_Position=projection*view*model*gl_Position;

  // transform the normal vector with the vertices
  mat3 NormalMatrix = mat3(transpose(inverse(model)));
  vec3 T, B, N;
  T = normalize(NormalMatrix * tangent);
  N = normalize(NormalMatrix * aNormal);
  T = normalize(T - dot(T, N) * N);
  B = cross(N, T);

  // Output TBN to fragment shader
  TBN=mat3(T, B, N);
}