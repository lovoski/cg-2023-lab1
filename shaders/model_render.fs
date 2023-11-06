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

  vec3 viewDir = normalize(viewPos-FragPos);
  vec3 lightDir = normalize(lightPos-FragPos);
  // replace v*r with h*n
  vec3 halfwayDir = normalize(lightDir+viewDir);

  vec3 diffuse;
  diffuse = light.diffuse*material.diffuse*(max(dot(lightDir, norm), 0.));

  vec3 specular;
  // use the spec parameter from spec map
  specular = light.specular*material.specular*objSpec*pow(max(dot(halfwayDir, norm), 0.), material.shininess);

  float dist = length(light.position - FragPos);
  float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist));

  vec3 reflectDir = reflect(-viewDir, norm);
  vec3 sky;
  sky = skylightIntensity*(texture(skybox, reflectDir).rgb);

  vec3 result=(ambient+diffuse+specular)*objColor*(attenuation+sky);

  FragColor=vec4(result,1.);

  // gamma correction
  FragColor.rgb=pow(FragColor.rgb,vec3(1./gamma));
}