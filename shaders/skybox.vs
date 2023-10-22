#version 460 core
layout(location=0)in vec3 aPos;

out vec3 TexCoords;

layout(std140,binding=0)uniform Object
{
  mat4 projection;
  mat4 view;
};

// uniform mat4 projection;
// uniform mat4 view;
uniform vec3 offset;

void main()
{
  TexCoords=aPos;
  vec4 pos=projection*view*vec4(aPos+offset,1.);
  gl_Position=pos.xyww;
}