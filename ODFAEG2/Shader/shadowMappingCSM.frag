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
    layout(offset=144) mat4 view;    
    layout(offset=208) vec3 lightDir;
    layout(offset=224) float farPlane;
} pc;
layout (std430, set = 0, binding = 1) buffer MaterialDataSSBO {
    MaterialData materialData[];
} materialDataBuffer[NB_PRIMITIVE_TYPES * MAX_FRAMES_IN_FLIGHT];
layout (std140, set = 0, binding = 2) uniform LightSpaceMatricesUBO {
    mat4 lightSpaceMatrices[NB_CASCADES+1];
} lightSpaceMatricesData[MAX_FRAMES_IN_FLIGHT];
layout (std140, set = 0, binding = 3) uniform cascadePlaneDistanceUBO {
    float cascadePlaneDistances[NB_CASCADES+1];
    int cascadeCount;   // number of
} cascadePlaneDistanceData[MAX_FRAMES_IN_FLIGHT];
layout(set = 0, binding = 4) uniform sampler2DArray shadowMap;
layout(set = 0, binding = 5) uniform sampler2D diffuseTextures[MAX_TEXTURES];
float shadowCalculation(vec3 fragPosWorldSpace)
{
    // select cascade layer
    
    vec4 fragPosViewSpace = pc.view * vec4(fragPosWorldSpace, 1.0);
    //debugPrintfEXT("Frag pos view space : %v3f", fragPosViewSpace);
    float depthValue = abs(fragPosViewSpace.z);

    int layer = -1;    
    //debugPrintfEXT("plane distances : %f,%f,%f,%f,%f", cascadePlaneDistanceData[currentFrame].cascadePlaneDistances[0],cascadePlaneDistanceData[currentFrame].cascadePlaneDistances[1],cascadePlaneDistanceData[currentFrame].cascadePlaneDistances[2],cascadePlaneDistanceData[currentFrame].cascadePlaneDistances[3],cascadePlaneDistanceData[currentFrame].cascadePlaneDistances[4]);
    
    for (int i = 0; i < NB_CASCADES; ++i)
    {    
        //debugPrintfEXT("depth value : %f", depthValue);    
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
    if (layer > 1) {
        debugPrintfEXT("layer %i ", layer);
    }
    vec4 fragPosLightSpace =  lightSpaceMatricesData[currentFrame].lightSpaceMatrices[layer] * vec4(fragPosWorldSpace, 1.0);
    //debugPrintfEXT("light space matrix : %v4f\n%v4f\n%v4f\n%v4f \nfragPosLightSpace%v4f\nfragPosWorldSpace%v3f", lightSpaceMatricesData[currentFrame].lightSpaceMatrices[layer][0],lightSpaceMatricesData[currentFrame].lightSpaceMatrices[layer][1],lightSpaceMatricesData[currentFrame].lightSpaceMatrices[layer][2],lightSpaceMatricesData[currentFrame].lightSpaceMatrices[layer][3], fragPosLightSpace,fragPosWorldSpace);
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords.xy = projCoords.xy * 0.5 + 0.5;
    //debugPrintfEXT("frag pos light space : %v4f, proj coords : %v3f", fragPosLightSpace, projCoords);

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (currentDepth < 0 || currentDepth > 1.0)
    {
        //debugPrintfEXT("current depth : > 1");
        return 0.0;
    }
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(normal);
    float bias = max(0.05 * (1.0 - dot(normal, pc.lightDir)), 0.005);
    const float biasModifier = 0.5f;
    if (layer == NB_CASCADES)
    {
        bias *= 1 / (pc.farPlane * biasModifier);
    }
    else
    {
        bias *= 1 / (cascadePlaneDistanceData[currentFrame].cascadePlaneDistances[layer] * biasModifier);
    }

    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    //debugPrintfEXT("projCoords : %v3f, texelSize : %v2f", vec3(projCoords.xy * texelSize, projCoords.z), texelSize);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
            //if (pcfDepth > 0)
            //debugPrintfEXT("coords : %v3f, pcf : %f", vec3(projCoords.xy + vec2(x, y) * texelSize, layer), pcfDepth);
            shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
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
    float shadow = shadowCalculation(fragPos);
    /*if (shadow < 1)
        debugPrintfEXT("shadow %f", shadow);*/
    frag_color = diffuse * (1 - shadow);
}
