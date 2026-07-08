#version 460
#extension GL_EXT_nonuniform_qualifier : enable
#define NB_PRIMITIVE_TYPES 6
#define MAX_FRAMES_IN_FLIGHT 2
layout(location = 0) in vec3 pos;
layout(location = 1) in vec4 fragColor;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 normal;
layout(location = 4) flat in int materialId;
layout(location = 5) flat in int primitiveType;
layout(location = 6) flat in int currentFrame;
layout(location = 0) out vec4 outColor;
layout (push_constant) uniform PushConsts {
    layout(offset = 224) vec4 cameraPos;        
    layout(offset = 240) uint imageIndex;
} pushConsts;
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
    int reflectable;
    int refractable;
};
layout (std430, set = 0, binding = 0) buffer MaterialDataSSBO {
    MaterialData materialData[];
} materialDataBuffer[NB_PRIMITIVE_TYPES * MAX_FRAMES_IN_FLIGHT];
layout (set = 0, binding = 1) uniform samplerCube sceneBox[];                                                       
void main() {
    MaterialData material = materialDataBuffer[primitiveType * MAX_FRAMES_IN_FLIGHT+currentFrame].materialData[materialId];
    vec4 reflectColor = vec4(1);
    vec3 i = (vec4(pos.xyz, 1) - pushConsts.cameraPos).xyz;
    if (material.reflectable == 1) {
        vec3 r = reflect (i, normalize(normal));
        reflectColor = texture(sceneBox[pushConsts.imageIndex], r);
    }
    float ratio = 1;
    if (material.materialType == 1) {
        ratio = 1.00 / 1.33;        
    } else if (material.materialType == 2) {
        ratio = 1.00 / 1.309;        
    } else if (material.materialType == 3) {
        ratio = 1.00 / 1.52;        
    } else if (material.materialType == 4) {
        ratio = 1.00 / 2.42;        
    }
    vec4 refractColor = vec4(1);
    if (material.refractable == 1) {
        vec3 r = refract (i, normalize(normal), ratio);
        refractColor = texture(sceneBox[pushConsts.imageIndex], r);
    }
    outColor = fragColor * reflectColor * refractColor;
}  