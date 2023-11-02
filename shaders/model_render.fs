#version 460 core
out vec4 FragColor;

in vec2 TexCoords;
// in vec3 Normal;

// layout(std140,binding=0)uniform Object{samplerCube skybox;};

// uniform sampler2D texture_diffuse1;
// uniform sampler2D texture_specular1;
// uniform sampler2D texture_normal1;
// uniform sampler2D texture_height1;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

struct Light{
  vec3 position;
  
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  
  float constant;
  float linear;
  float quadratic;
};

const int NR_LIGHTS = 10;
uniform Light lights[NR_LIGHTS];

uniform float skylightIntensity;
uniform vec3 viewPos;

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
  // retrieve data from gbuffer
  vec3 FragPos = texture(gPosition, TexCoords).rgb;
  vec3 Normal = texture(gNormal, TexCoords).rgb;
  vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
  float Specular = texture(gAlbedoSpec, TexCoords).a;

  // then calculate lighting as usual
  vec3 lighting  = Diffuse * 0.1; // hard-coded ambient component
  vec3 viewDir  = normalize(viewPos - FragPos);
  for(int i = 0; i < NR_LIGHTS; ++i)
  {
    // diffuse
    vec3 lightDir = normalize(lights[i].position - FragPos);
    vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lights[i].diffuse;
    // specular
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), material.shininess);
    vec3 specular = lights[i].specular * spec * Specular;
    // attenuation
    float dist = length(lights[i].position - FragPos);
    float attenuation = 1.0 / (1.0 + lights[i].linear * dist + lights[i].quadratic * dist * dist);

    vec3 reflectDir = reflect(-viewDir, Normal);
    vec3 skyReflection = skylightIntensity*(texture(skybox, reflectDir).rgb);

    diffuse *= (attenuation+skyReflection);
    specular *= (attenuation+skyReflection);
    lighting += diffuse + specular;
  }
  FragColor = vec4(lighting, 1.0);
  FragColor.rgb=pow(FragColor.rgb,vec3(1./gamma));
}