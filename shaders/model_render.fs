#version 460 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in mat3 TBN;
in vec4 FragPosLightSpace;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;

uniform sampler2D depthMap;

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

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    vec3 lightPos=light.position;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(depthMap, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(TBN*(vec3(texture(texture_normal1,TexCoords))*2.0-1.0));
    vec3 lightDir = normalize(lightPos - FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(depthMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(depthMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
    return shadow;
}

void main()
{
  // only one light source
  // obtain color from texture and texture coordinates
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
  vec3 F0_=mix(F0,objColor,objSpec);
  vec3 F = fresnelSchlick(dot(norm, viewDir), F0_);
  vec3 sky;
  sky = skylightIntensity*(objH.r+F*.3)*(texture(skybox, reflectDir).rgb);

  float shadow = ShadowCalculation(FragPosLightSpace);

  vec3 result=(diffuse+specular)*objColor*(attenuation+sky)*(ambient+1.0-shadow);

  FragColor=vec4(result,1.);

  // gamma correction
  FragColor.rgb=pow(FragColor.rgb,vec3(1./gamma));
}