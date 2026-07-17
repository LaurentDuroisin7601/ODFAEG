#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_debug_printf : enable
#define NB_PRIMITIVE_TYPES 6
#define MAX_FRAMES_IN_FLIGHT 2
const uint MAX_TEXTURES = 1024;
layout (push_constant) uniform PushConstant {
    layout(offset=136) int currentImageIndex;
} pc;
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
layout (std430, set = 0, binding = 2) buffer MaterialDataSSBO {
    MaterialData materialData[];
} materialDataBuffer[NB_PRIMITIVE_TYPES * MAX_FRAMES_IN_FLIGHT];
layout(set = 0, binding = 3) uniform sampler2D diffuseTextures[MAX_TEXTURES];
layout(set = 1, binding = 0) uniform sampler2D specularTextures[MAX_TEXTURES];
layout(set = 2, binding = 0) uniform sampler2D normalTextures[MAX_TEXTURES];
layout(set = 3, binding = 0) uniform sampler2D metalnessTextures[MAX_TEXTURES];
layout(set = 4, binding = 0) uniform sampler2D roughnessTextures[MAX_TEXTURES];
layout(set = 5, binding = 0) uniform sampler2D aoTextures[MAX_TEXTURES];
layout(set = 6, binding = 0) uniform sampler2D emissiveTextures[MAX_TEXTURES];
layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) flat in int materialID;
layout(location = 4) flat in int primitiveType;
layout(location = 5) flat in int currentFrame;
layout(location = 0) out vec4 outColor;
void main() {
    MaterialData mat = materialDataBuffer[primitiveType * MAX_FRAMES_IN_FLIGHT+currentFrame].materialData[materialID];
    vec2 uv = fragTexCoord;
    // --- Diffuse ---
    vec4 diffuse = fragColor;
    //debugPrintfEXT("diffuse %i", mat.diffuseTextureIndex);
    if (mat.diffuseTextureIndex > 0 && mat.diffuseTextureIndex < MAX_TEXTURES) {
        diffuse *= texture(diffuseTextures[mat.diffuseTextureIndex-1], uv);
    }
    // --- Normal map ---
    vec3 N = normalize(normal);
    if (mat.normalTextureIndex > 0 && mat.normalTextureIndex < MAX_TEXTURES) {
        vec3 nmap = texture(normalTextures[mat.normalTextureIndex-1], uv).xyz * 2.0 - 1.0;
        N = normalize(nmap);
    }
    // --- Specular ---
    float specularStrength = 1.0;
    if (mat.specularTextureIndex > 0 && mat.specularTextureIndex < MAX_TEXTURES) {
        //debugPrintfEXT("Specular");
        specularStrength = texture(specularTextures[mat.specularTextureIndex-1], uv).r;
    }
    // --- AO ---
    float ao = 1.0;
    if (mat.aoTextureIndex > 0 && mat.aoTextureIndex < MAX_TEXTURES) {
        //debugPrintfEXT("AO");
        ao = texture(aoTextures[mat.aoTextureIndex-1], uv).r;
    }
    // --- Emissive ---
    vec4 emissive = vec4(0.0);
    if (mat.emissiveTextureIndex > 0 && mat.emissiveTextureIndex < MAX_TEXTURES) {
        //debugPrintfEXT("Emissive");
        emissive = vec4(texture(emissiveTextures[mat.emissiveTextureIndex-1], uv).rgb, 0);
    }
	// --- Éclairage simple ---
    vec3 L = normalize(vec3(0.5, 1.0, 0.3));
    float diff = max(dot(-N, L), 0.0);
    // Specular simple
    vec3 V = normalize(vec3(0,0,1));
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), 32.0) * specularStrength;
    //debugPrintfEXT("texture index : %i, normal : %v3f, fragTexCoord %v2f, color : %v4f, fragColor : %v4f", mat.diffuseTextureIndex, normal, fragTexCoord, diffuse * diff * ao + spec + emissive, fragColor);
    //debugPrintfEXT("normal : %v3f", N);
    outColor = diffuse/* * diff * ao + spec + emissive*/;
   /* if (outColor.a == 0)
        debugPrintfEXT("out color : %v4f", outColor);*/
};