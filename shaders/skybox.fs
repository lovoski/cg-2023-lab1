#version 460 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

uniform bool enableSkyLight;

void main()
{
  if (enableSkyLight) {
    FragColor=texture(skybox,TexCoords);
  } else {
    FragColor=vec4(0.,0.,0.,1.);
  }
  // FragColor=vec4(1.,1.,1.,1.);
}