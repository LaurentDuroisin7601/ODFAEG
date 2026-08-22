#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_debug_printf : enable
#define MAX_FRAMES_IN_FLIGHT 2
#define MAX_PRIMITIVE_TYPES 6
#define MAX_TEXTURES 1024
#define MAX_BONES_INFLUENCE 4
struct Vertex {
    vec3 position; ///< 3D position of the vertex
    uint color; ///< Color of the vertex
    vec2 texCoords; ///< Coordinates of the texture's pixel to map to the vertex
    vec3 normal;
    //bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONES_INFLUENCE];
    //weights from each bone
    float m_Weights[MAX_BONES_INFLUENCE];
    int submeshId;
};
struct GeometryOffset {
    uint vertexOffset;
    uint indexOffset;
    uint materialOffset;
};
struct MaterialData {   
    /*vec2 uvScale;
    vec2 uvOffset;*/
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
layout (binding = 0, set =  1) buffer vertexBuffer {
    Vertex vertices[];
} verticesData[MAX_PRIMITIVE_TYPES];
layout (binding = 1, set = 1) buffer indexBuffer {
    uint indexes[];
} indexesData[MAX_PRIMITIVE_TYPES];
layout (binding = 2, set = 1) buffer geomBuffer {
    GeometryOffset geomOffsets[];
};
layout (binding = 3, set = 1) buffer materialBuffer {
    MaterialData materials[];
};
layout (binding = 4, set = 1) uniform sampler2D diffuseTextures[MAX_TEXTURES];
struct RayPayload {
    vec4 color;
};
layout(location = 0) rayPayloadInEXT RayPayload payload;
hitAttributeEXT vec2 baryCoords;
vec4 unpackColor(uint c)
{
    float r = float((c >> 0)  & 0xFFu) / 255.0;
    float g = float((c >> 8)  & 0xFFu) / 255.0;
    float b = float((c >> 16) & 0xFFu) / 255.0;
    float a = float((c >> 24) & 0xFFu) / 255.0;

    return vec4(r, g, b, a);
}

void main() {
    GeometryOffset geomOffs = geomOffsets[gl_InstanceCustomIndexEXT];
    int primID = gl_PrimitiveID;
    uint i1 = indexesData[3].indexes[geomOffs.indexOffset + primID * 3 + 0];
    uint i2 = indexesData[3].indexes[geomOffs.indexOffset + primID * 3 + 1];
    uint i3 = indexesData[3].indexes[geomOffs.indexOffset + primID * 3 + 2];
    vec4 c1 = unpackColor(verticesData[3].vertices[geomOffs.vertexOffset + i1].color);
    vec4 c2 = unpackColor(verticesData[3].vertices[geomOffs.vertexOffset + i2].color);
    vec4 c3 = unpackColor(verticesData[3].vertices[geomOffs.vertexOffset + i3].color);
    MaterialData material = materials[geomOffs.materialOffset];
    vec2 ct1 = verticesData[3].vertices[geomOffs.vertexOffset + i1].texCoords.xy;
    vec2 ct2 = verticesData[3].vertices[geomOffs.vertexOffset + i2].texCoords.xy;
    vec2 ct3 = verticesData[3].vertices[geomOffs.vertexOffset + i3].texCoords.xy;
    float u = baryCoords.x;
    float v = baryCoords.y;
    float w = 1.0 - u - v;
    vec4 color = w * c1 + u * c2 + v * c3;
    vec2 tc = w * ct1 + u * ct2 + v * ct3;
    uint textureIndex = material.diffuseTextureIndex;
    
    payload.color = /*(textureIndex > 0) ? color * texture(diffuseTextures[textureIndex-1], tc) :*/ color;
    /*if (payload.color.r > 1 || payload.color.g > 1 || payload.color.b > 1 || payload.color.a > 1
    || payload.color.r < 0 || payload.color.g < 0 || payload.color.b < 0 || payload.color.a < 0) {
        debugPrintfEXT("color : %v4f, %v4f, %v4f, %v4f %v4f", c1, c2, c3, color, texture(diffuseTextures[textureIndex-1], tc));
    }*/
}