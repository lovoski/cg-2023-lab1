#version 460 core
out vec4 FragColor;

in vec2 TexCoords;
// in vec3 Normal;
in vec3 FragPos;
in mat3 TBN;

// layout(std140,binding=0)uniform Object{samplerCube skybox;};

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;

struct Light{
  vec3 position;
  
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  
  float constant;
  float linear;
  float quadratic;
};

uniform Light light;

uniform float skylightIntensity;

// view
uniform vec3 viewPos;

// Color
// uniform vec3 ambient;

// material
struct Material{
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
};

uniform Material material;

uniform samplerCube skybox;

uniform float gamma;

uniform bool existHeigTex;

uniform vec3 F0;

vec3 fresnelSchlick(float cosTheta,vec3 F0)
{
  return F0+(1.-F0)*pow(1.-cosTheta,5.);
}

void main()
{
  vec3 objColor=vec3(texture(texture_diffuse1,TexCoords));
  vec3 objSpec=vec3(texture(texture_specular1,TexCoords));
  vec3 objNorm=vec3(texture(texture_normal1,TexCoords));
  vec3 objheig=vec3(0.);
  if(existHeigTex){
    objheig=vec3(texture(texture_height1,TexCoords));
  }
  
  vec3 objH=max(objheig,.05);
  
  vec3 norm=normalize(TBN*normalize(objNorm*2.-1.));
  vec3 lightPos=light.position;
  
  vec3 ambient=light.ambient*material.ambient;
  
  // TODO: view direction: lightDir, viewDir and halfwayDir
  
  // TODO: scatter term
  vec3 diffuse;
  
  // TODO: specular reflect term
  vec3 specular;
  
  // TODO: attenuation
  float attenuation;
  
  // TODO: SkyBox reflect term
  // NOTE: you should add fresnel reflection here!
  // vec3 F0=vec3(.04);
  vec3 F0_=mix(F0,objColor,objSpec);
  vec3 F;
  vec3 sky;
  sky=(objH.r+F*.3)*sky;
  
  vec3 result=(ambient+diffuse+specular)*objColor*(attenuation+sky);
  // result=sky;
  
  FragColor=vec4(result,1.);
  
  // TODO: do gamma correction here
  // gamma
  FragColor.rgb=pow(FragColor.rgb,vec3(1./gamma));
}