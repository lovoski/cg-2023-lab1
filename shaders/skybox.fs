#version 460 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;
uniform int skyboxOn;

void main()
{
  if (skyboxOn == 1) {
    FragColor=texture(skybox,TexCoords);
  }
  // FragColor=vec4(1.,1.,1.,1.);
}