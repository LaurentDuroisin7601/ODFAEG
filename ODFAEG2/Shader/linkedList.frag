#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_debug_printf : enable
#extension GL_EXT_multiview : enable
#define MAX_TEXTURES 1024
#define MAX_FRAMES_IN_FLIGHT 2
#define NB_PRIMITIVE_TYPES 6
layout (early_fragment_tests) in;
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
layout (push_constant) uniform PushConstant {
    layout (offset=136) uint maxNodes;
    layout (offset=140) int currentImageIndex;
} pc;
layout(set = 0, binding = 1, r32ui) uniform coherent uimage2D headPointers[MAX_FRAMES_IN_FLIGHT];
layout (std430, set = 0, binding = 2) buffer MaterialDataSSBO {
    MaterialData materialData[];
} materialDataBuffer[NB_PRIMITIVE_TYPES * MAX_FRAMES_IN_FLIGHT];
layout(std430, set = 0, binding = 3) buffer LinkedLists {
  NodeType nodes[];
} linkedListData[MAX_FRAMES_IN_FLIGHT];
layout(std430, set = 0, binding = 4) buffer CounterSSBO {
  uint count;
} countData[MAX_FRAMES_IN_FLIGHT];
layout(set = 0, binding = 5) uniform sampler2D diffuseTextures[MAX_TEXTURES];
layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) flat in int v_DrawID;
layout(location = 4) flat in int primitiveType;
layout(location = 5) flat in int currentFrame;
layout(location = 0) out vec4 outColor;
void main() {
    //debugPrintfEXT("current frame : %i, primitive type : %i", currentFrame, primitiveType);

    MaterialData mat = materialDataBuffer[primitiveType * MAX_FRAMES_IN_FLIGHT+currentFrame].materialData[v_DrawID];

    // --- Diffuse ---
    vec2 uv = fragTexCoord;
    vec4 diffuse = fragColor;
    if (mat.diffuseTextureIndex > 0 && mat.diffuseTextureIndex < MAX_TEXTURES) {
        diffuse *= texture(diffuseTextures[mat.diffuseTextureIndex-1], uv);
    }
    uint nodeIdx = atomicAdd(countData[currentFrame].count, 1);
    if (nodeIdx < pc.maxNodes) {
         uint prevHead = imageAtomicExchange(headPointers[currentFrame], ivec2(gl_FragCoord.xy), nodeIdx);
         linkedListData[currentFrame].nodes[nodeIdx].color = diffuse;
         linkedListData[currentFrame].nodes[nodeIdx].depth = gl_FragCoord.z;
         linkedListData[currentFrame].nodes[nodeIdx].next = prevHead;
    }
    //discard;
}