#version 440 
#extension GL_EXT_debug_printf : enable
#extension GL_EXT_multiview : enable
#define MAX_TEXTURES 1024
#define NB_PRIMITIVE_TYPES 6
#define MAX_FRAMES_IN_FLIGHT 2
#define NB_CASCADES 4
struct NodeType {
    vec4 color;
    float depth;
    uint next;
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
layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 normal;
layout (location = 3) in flat int v_DrawID;
layout (location = 4) in flat int primitiveType;
layout (location = 5) in flat int currentFrame;
layout (push_constant) uniform PushConstant {
    layout(offset=8) int maxNodes; 
} pc;
layout (std430, set = 0, binding = 2) buffer MaterialDataSSBO {
    MaterialData materialData[];
} materialDataBuffer[NB_PRIMITIVE_TYPES * MAX_FRAMES_IN_FLIGHT];
layout (set = 0, binding = 3, r32ui) uniform coherent uimage2D headPointers[MAX_FRAMES_IN_FLIGHT*(NB_CASCADES+1)];
layout (std430, set = 0, binding = 4) buffer nodeCountSSBO {
    uint count;
} countData[MAX_FRAMES_IN_FLIGHT*(NB_CASCADES+1)];
layout (std430, set = 0, binding = 5) buffer linkedListSSBO {
    NodeType nodes[];
} linkedListData[MAX_FRAMES_IN_FLIGHT*(NB_CASCADES+1)];
layout (set = 0, binding = 6) uniform sampler2D diffuseTextures[MAX_TEXTURES];
void main()
{   
    MaterialData mat = materialDataBuffer[primitiveType * MAX_FRAMES_IN_FLIGHT+currentFrame].materialData[v_DrawID];
    // --- Diffuse ---
    vec2 uv = fragTexCoord;
    vec4 diffuse = fragColor;
    if (mat.diffuseTextureIndex > 0 && mat.diffuseTextureIndex < MAX_TEXTURES) {
        diffuse *= texture(diffuseTextures[mat.diffuseTextureIndex-1], uv);
    }
    uint nodeIdx = atomicAdd(countData[gl_ViewIndex*MAX_FRAMES_IN_FLIGHT+currentFrame].count, 1);
    if (nodeIdx < pc.maxNodes) {
         uint prevHead = imageAtomicExchange(headPointers[gl_ViewIndex*MAX_FRAMES_IN_FLIGHT+currentFrame], ivec2(gl_FragCoord.xy), nodeIdx);
         linkedListData[gl_ViewIndex*MAX_FRAMES_IN_FLIGHT+currentFrame].nodes[nodeIdx].color = diffuse;
         linkedListData[gl_ViewIndex*MAX_FRAMES_IN_FLIGHT+currentFrame].nodes[nodeIdx].depth = gl_FragCoord.z;
         linkedListData[gl_ViewIndex*MAX_FRAMES_IN_FLIGHT+currentFrame].nodes[nodeIdx].next = prevHead;
    }      
}