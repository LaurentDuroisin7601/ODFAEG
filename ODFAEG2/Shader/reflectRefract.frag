#version 460
#extension GL_EXT_debug_printf : enable
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
layout (std430, set = 0, binding = 0) buffer MaterialDataSSBO {
    MaterialData materialData[];
} materialDataBuffer[MAX_FRAMES_IN_FLIGHT*NB_PRIMITIVE_TYPES];
layout (set = 0, binding = 1) uniform samplerCube sceneBox;                                                       
void main() {
    MaterialData material = materialDataBuffer[primitiveType*MAX_FRAMES_IN_FLIGHT+currentFrame].materialData[materialId];
    vec4 reflectColor = vec4(1);    
    vec3 i = vec3(vec4(pos.xyz, 1) - pc.cameraPos);
    //debugPrintfEXT("frag pos : %v3f, camera pos : %v4f, i : %v3f", pos, pc.cameraPos, i);
    /*if (normal.x != 0 || normal.y != 0 || normal.z != 0)
        debugPrintfEXT("i : %v3f, normal : %v3f", i, normalize(normal));*/
    //debugPrintfEXT("material id : %i, reflectable %i, refractable %i", material.materialId, material.reflectable, material.refractable);
    if (material.reflectable == 1) {
        vec3 r = reflect (i, normalize(normal));
        reflectColor = texture(sceneBox, r);
        
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
        refractColor = texture(sceneBox, r);        
    }
    /*if ((reflectColor.r != 0 || reflectColor.g != 0 && reflectColor.b != 0) && (refractColor.r != 0 || refractColor.g != 0 && refractColor.b != 0))
        debugPrintfEXT("ratio : %f, refract color : %v4f", ratio, refractColor);*/
    outColor = mix(reflectColor, refractColor, refractColor.a);
}  