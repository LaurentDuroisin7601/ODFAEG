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
    layout(offset=144) vec3 lightPos;    
    layout(offset=160) float far_plane;    
} pc;
layout (std430, set = 0, binding = 1) buffer MaterialDataSSBO {
    MaterialData materialData[];
} materialDataBuffer[NB_PRIMITIVE_TYPES * MAX_FRAMES_IN_FLIGHT];
layout(set = 0, binding = 2) uniform samplerCube depthMap;
layout(set = 0, binding = 3) uniform sampler2D diffuseTextures[MAX_TEXTURES];
float shadowCalculation(vec3 fragPos)
{
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - lightPos;
    // use the light to fragment vector to sample from the depth map    
    float closestDepth = texture(depthMap, fragToLight).r;
    // it is currently in linear range between [0,1]. Re-transform back to original value
    closestDepth *= far_plane;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // now test for shadows
    float bias = 0.05; 
    float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}  
void main()
{           
    MaterialData mat = materialDataBuffer[primitiveType * MAX_FRAMES_IN_FLIGHT+currentFrame].materialData[v_DrawID];
    //debugPrintfEXT("draw");
    vec4 diffuse = fragColor;
    if (mat.diffuseTextureIndex > 0)
        diffuse *= texture(diffuseTextures[mat.diffuseTextureIndex-1], fragTexCoord);    
    // calculate shadow
    float shadow = shadowCalculation(fragPos);  
    frag_color = diffuse * (1.0 - shadow);
}  