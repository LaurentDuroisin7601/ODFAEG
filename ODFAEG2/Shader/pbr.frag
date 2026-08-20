#version 460
#define NB_PRIMITIVE_TYPES 6
#define MAX_FRAMES_IN_FLIGHT 2 
#define MAX_TEXTURES 24
struct Light {
    vec3 lightPos;
    vec3 lightColor;
};
layout (push_constant) uniform PushConstant {
    layout (offset = 144) vec4 camPos;        
} pc;
struct MaterialData {
    uint diffuseTextureIndex;
    uint specularTextureIndex;
    uint normalTextureIndex;
    uint metalnessTextureIndex;
    uint roughnessTextureIndex;
    uint aoTextureIndex;
    uint emissiveTextureIndex;
    uint materialType;
    uint materialSet;
    uint nbVertices;
    uint nbIndexes;
    int instanceGroupId;
    uint vertsInstanceSet;
    uint materialId;
    uint nbBuffers;
    int reflectable;
    int refractable;      
};
layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec4 fragColor;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 normal;
layout(location = 4) flat in int materialID;
layout(location = 5) flat in int primitiveType;
layout(location = 6) flat in int currentFrame;
layout (location = 0) out vec4 outColor;
layout (std430, set = 0, binding = 1) buffer MaterialDataSSBO {
    MaterialData materialData[];
} materialDataBuffer[NB_PRIMITIVE_TYPES * MAX_FRAMES_IN_FLIGHT];
layout (std430, set = 0, binding = 2) buffer LightMatricesSSBO {
    Light currentLight;   
} lightData[MAX_FRAMES_IN_FLIGHT];
layout(set = 0, binding = 3) uniform samplerCube irradianceMap;
layout(set = 0, binding = 4) uniform samplerCubeArray prefilterMap;
layout(set = 0, binding = 5) uniform sampler2D brdfLUT;
layout(set = 0, binding = 6) uniform sampler2D diffuseTextures[MAX_TEXTURES];
layout(set = 1, binding = 0) uniform sampler2D specularTextures[MAX_TEXTURES];
layout(set = 2, binding = 0) uniform sampler2D normalTextures[MAX_TEXTURES];
layout(set = 3, binding = 0) uniform sampler2D metalnessTextures[MAX_TEXTURES];
layout(set = 4, binding = 0) uniform sampler2D roughnessTextures[MAX_TEXTURES];
layout(set = 5, binding = 0) uniform sampler2D aoTextures[MAX_TEXTURES];
layout(set = 6, binding = 0) uniform sampler2D emissiveTextures[MAX_TEXTURES];
const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal 
// mapping the usual way for performance anyways; I do plan make a note of this 
// technique somewhere later in the normal mapping tutorial.
vec3 getNormalFromMap()
{
    MaterialData material = materialDataBuffer[primitiveType * MAX_FRAMES_IN_FLIGHT + currentFrame].materialData[materialID];
    vec3 tangentNormal = vec3(0, 1, 0);
    if (material.normalTextureIndex > 0 && material.normalTextureIndex < MAX_TEXTURES) {
        tangentNormal = texture(normalTextures[material.normalTextureIndex-1], fragTexCoord).xyz * 2.0 - 1.0;
    }
    vec3 Q1  = dFdx(fragPos);
    vec3 Q2  = dFdy(fragPos);
    vec2 st1 = dFdx(fragTexCoord);
    vec2 st2 = dFdy(fragTexCoord);

    vec3 N   = normalize(normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}   
void main() {
    // material properties
    MaterialData mat = materialDataBuffer[primitiveType * MAX_FRAMES_IN_FLIGHT + currentFrame].materialData[materialID];
    vec3 albedo = fragColor.xyz;
    if (mat.diffuseTextureIndex > 0 && mat.diffuseTextureIndex < MAX_TEXTURES) {
        albedo *= pow(texture(diffuseTextures[mat.diffuseTextureIndex-1], fragTexCoord).rgb, vec3(2.2));
    } 
    float metallic = 1.0;
    if (mat.metalnessTextureIndex > 0 && mat.metalnessTextureIndex < MAX_TEXTURES) {
        metallic = texture(metalnessTextures[mat.metalnessTextureIndex-1], fragTexCoord).r;
    }
    float roughness = 1.0;
    if (mat.roughnessTextureIndex > 0 && mat.roughnessTextureIndex < MAX_TEXTURES) {
        roughness = texture(roughnessTextures[mat.roughnessTextureIndex-1], fragTexCoord).r; 
    }    
    float ao = 1.0;
    if (mat.aoTextureIndex > 0 && mat.aoTextureIndex < MAX_TEXTURES) {
        //debugPrintfEXT("AO");
        ao = texture(aoTextures[mat.aoTextureIndex-1], fragTexCoord).r;
    }
    // --- Emissive ---
    vec3 emissive = vec3(0.0);
    if (mat.emissiveTextureIndex > 0 && mat.emissiveTextureIndex < MAX_TEXTURES) {
        //debugPrintfEXT("Emissive");
        emissive = texture(emissiveTextures[mat.emissiveTextureIndex-1], fragTexCoord).rgb;
    }
    vec3 N = getNormalFromMap();
    vec3 V = normalize(pc.camPos.xyz - fragPos);
    vec3 R = reflect(-V, N); 
    Light light = lightData[currentFrame].currentLight;
    vec3 L = normalize(light.lightPos - fragPos);    
    vec3 H = normalize(V + L);
    float distance = length(light.lightPos - fragPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = light.lightColor * attenuation;
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(N, V, L, roughness);    
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);        
    
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
    vec3 specular = numerator / denominator;
    
        // kS is equal to Fresnel
    vec3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals 
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - metallic;	                
        
    // scale light by NdotL
    float NdotL = max(dot(N, L), 0.0);        

    // add to outgoing radiance Lo
    Lo += (kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    
    
    // ambient lighting (we now use IBL as the ambient term)
    F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    
    kS = F;
    kD = 1.0 - kS;
    kD *= 1.0 - metallic;	  
    
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse      = irradiance * albedo;
    
    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = texture(prefilterMap, vec4(R,  roughness * MAX_REFLECTION_LOD)).rgb;    
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * ao;
    
    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2));
    color += emissive; 

    outColor = vec4(color , 1.0);
}