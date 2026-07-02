#version 460
#extension GL_EXT_debug_printf : enable
#define NB_CASCADES 4
#define MAX_TEXTURES 1024
#define NB_PRIMITIVE_TYPES 6
#define MAX_FRAMES_IN_FLIGHT 2
layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) flat in int v_DrawID;
layout(location = 4) flat in int primitiveType;
layout(location = 5) flat in int currentFrame;
layout(location = 6) in vec3 fragPos;
layout(location = 0) out vec4 frag_color;
#define NB_CASCADES 4
struct LightSpaceMatrix {
    mat4 lightSpaceMatrices[NB_CASCADES+1];
};
struct DirLight {
    mat4 view;
    vec3 dir;
    float far_plane;
};
struct PointLight {
    vec3 pos;
    float far_plane;
};
struct MaterialData {
    vec2 uvScale;
    vec2 uvOffset;
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
    uint padding;
};
layout (push_constant) uniform PushConstant { 
    layout(offset=144) uint view;     
    layout(offset=208) uint nbDirLights;
    layout(offset=212) uint nbPointLights; 
} pc;
layout (std430, set = 0, binding = 1) buffer MaterialDataSSBO {
    MaterialData materialData[];
} materialDataBuffer[NB_PRIMITIVE_TYPES * MAX_FRAMES_IN_FLIGHT];
layout (std430, set = 0, binding = 2) buffer LightSpaceMatricesSSBO {
    LightSpaceMatrix lightSpaceMatrices[];
} lightSpaceMatricesData[MAX_FRAMES_IN_FLIGHT];
layout (std430, set = 0, binding = 3) uniform cascadePlaneDistanceSSBO {
    float cascadePlaneDistances[NB_CASCADES+1];
    int cascadeCount;   // number of
} cascadePlaneDistanceData[MAX_FRAMES_IN_FLIGHT];
layout(set = 0, binding = 4) uniform buffer DirLightSSBO {
    DirLight dirLights[];
} dirLightData[MAX_FRAMES_IN_FLIGHT];
layout(set = 0, binding = 5) uniform buffer DirLightSSBO {
    PointLight pointLights[];
} pointLightData[MAX_FRAMES_IN_FLIGHT];
layout(set = 0, binding = 6) uniform sampler2DArray shadowMap;
layout(set = 0, binding = 7) uniform samplerCube depthMap;
layout(set = 0, binding = 8) uniform sampler2D diffuseTextures[MAX_TEXTURES];
float shadowCalculationDir(vec3 fragPosWorldSpace)
{
    // select cascade layer 
    float shadow = 0.0;  
    for(int l = 0; l < pc.nbDirLights; l++) {
        vec4 fragPosViewSpace = dirLightData[currentFrame].dirLights[l].view * vec4(fragPosWorldSpace, 1.0);    
        float depthValue = abs(fragPosViewSpace.z);
        int layer = -1; 
        for (int i = 0; i < NB_CASCADES; ++i)
        {       
            if (depthValue < cascadePlaneDistanceData[currentFrame].cascadePlaneDistances[i])
            {
                
                layer = i;
                break;
            }
        }
        if (layer == -1)
        {
            layer = NB_CASCADES;
        } 
        vec4 fragPosLightSpace =  lightSpaceMatricesData[currentFrame].lightSpaceMatrices[l].lightSpaceMatrices[layer] * vec4(fragPosWorldSpace, 1.0);
        vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
        projCoords.xy = projCoords.xy * 0.5 + 0.5;
        // get depth of current fragment from light's perspective
        float currentDepth = projCoords.z;
        // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
        if (currentDepth < 0 || currentDepth > 1.0)
        {            
            return 0.0;
        }
        // calculate bias (based on depth map resolution and slope)
        vec3 normal = normalize(normal);
        float bias = max(0.05 * (1.0 - dot(normal,dirLightData[currentFrame].dirLights[l].lightDir)), 0.005);
        const float biasModifier = 0.5f;
        if (layer == NB_CASCADES)
        {
            bias *= 1 / (dirLightData[currentFrame].dirLights[l].far_plane * biasModifier);
        }
        else
        {
            bias *= 1 / (cascadePlaneDistanceData[currentFrame].cascadePlaneDistances[layer] * biasModifier);
        }
        // PCF
        float currentLightShadow = 0.0;
        vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));        
        for(int x = -1; x <= 1; ++x)
        {
            for(int y = -1; y <= 1; ++y)
            {
                float pcfDepth = texture(shadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;               
                currentLightShadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
            }
        }
        shadow = max(currentLightShadow, shadow);
    }     
    shadow /= 9.0;
    return shadow;
}
float shadowCalculationPoint(vec3 fragPos)
{
    // get vector between fragment position and light position
    // get vector between fragment position and light position
    float shadow = 0.0;
    for(int l = 0; l < pc.nbPointLights; l++) {
        vec3 fragToLight = fragPos - pointLightData[currentFrame].poointLights[l].lightPos;
        // use the light to fragment vector to sample from the depth map    
        float closestDepth = texture(depthMap, fragToLight).r;
        // it is currently in linear range between [0,1]. Re-transform back to original value
        closestDepth *= pointLightData[currentFrame].poointLights[l].far_plane;
        // now get current linear depth as the length between the fragment and light position
        float currentDepth = length(fragToLight);
        // now test for shadows
        float bias = 0.05; 
        vec3 lightDir = normalize(pc.lightPos - fragPos);
        float diff = max(dot(lightDir, normalize(normal)), 0.0); 
        //debugPrintfEXT("diff : %f", diff);   
        float currentLightShadow = currentDepth -  bias > closestDepth ? min(1.0, 1.0 * diff) : 0.0; 
        shadow = max(currentLightShadow, shadow);  
    } 
    return shadow;
}  
void main()
{
    MaterialData mat = materialDataBuffer[primitiveType * MAX_FRAMES_IN_FLIGHT+currentFrame].materialData[v_DrawID];
    //debugPrintfEXT("draw");
    vec4 diffuse = fragColor;
    if (mat.diffuseTextureIndex > 0)
        diffuse *= texture(diffuseTextures[mat.diffuseTextureIndex-1], fragTexCoord);
    vec3 normal = normalize(normal);
    // calculate shadow
    float shadowDir = shadowCalculationDir(fragPos);
    float shadowPoint = shadowCalculationPoint(fragPos);
    
    float shadow = max(shadowDir, shadowPoint);
    /*if (shadowDir > 0 || shadowPoint > 0)
        debugPrintfEXT("shadow dir %f", shadow); */     
    frag_color = vec4(vec3(diffuse) * (1 - shadow), diffuse.a);
}
