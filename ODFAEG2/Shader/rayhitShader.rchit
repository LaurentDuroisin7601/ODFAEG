#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_debug_printf : enable
#define MAX_FRAMES_IN_FLIGHT 2
#define MAX_PRIMITIVE_TYPES 6
#define MAX_TEXTURES 1024
#define MAX_BONES_INFLUENCE 4
#define NB_CASCADES 4
const float epsilon = 0.001;
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
    uint tlasID;
};
struct Material {   
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
    int opaque; 
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
    Material materials[];
};
layout (binding = 8, set = 1) uniform sampler2D diffuseTextures[MAX_TEXTURES];
layout(set = 2, binding = 0) uniform sampler2D specularTextures[MAX_TEXTURES];
layout(set = 3, binding = 0) uniform sampler2D normalTextures[MAX_TEXTURES];
layout(set = 4, binding = 0) uniform sampler2D metalnessTextures[MAX_TEXTURES];
layout(set = 5, binding = 0) uniform sampler2D roughnessTextures[MAX_TEXTURES];
layout(set = 6, binding = 0) uniform sampler2D aoTextures[MAX_TEXTURES];
layout(set = 7, binding = 0) uniform sampler2D emissiveTextures[MAX_TEXTURES];
struct TransportRayPayload {
    bool lastBounce;
    int rayType;
    vec4 finalColor;
    vec4 primaryColor;
    vec4 secondaryColor;
    vec4 transmitionColor;
    vec4 reflectColor;
    vec4 refractColor;
    vec3 origin;
    vec3 direction;
    vec3 R;
    vec3 T; 
    bool reflectable;
    bool refractable;
    bool transmissive;  
    int localASID; 
    vec3 normal;
    Material parentMaterial; 
    vec4 localLightning;
    vec3 lightPos;
    int lightId;        
    vec4 lightColor; 
    vec4 shadowColor; 
    mat4 dirLightSpace[NB_CASCADES+1];
    mat4 pointLightSpace[6];
};
layout(location = 0) rayPayloadInEXT TransportRayPayload transport;
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
    Material material = materials[geomOffs.materialOffset];
    vec2 ct1 = verticesData[3].vertices[geomOffs.vertexOffset + i1].texCoords.xy;
    vec2 ct2 = verticesData[3].vertices[geomOffs.vertexOffset + i2].texCoords.xy;
    vec2 ct3 = verticesData[3].vertices[geomOffs.vertexOffset + i3].texCoords.xy;
    vec3 v1 = verticesData[3].vertices[geomOffs.vertexOffset + i1].position;
    vec3 v2 = verticesData[3].vertices[geomOffs.vertexOffset + i2].position;
    vec3 v3 = verticesData[3].vertices[geomOffs.vertexOffset + i3].position;
    float u = baryCoords.x;
    float v = baryCoords.y;
    float w = 1.0 - u - v;
    vec4 color = w * c1 + u * c2 + v * c3;
    vec2 tc = w * ct1 + u * ct2 + v * ct3;
    uint textureIndex = material.diffuseTextureIndex;  
    vec4 albedo = (textureIndex > 0) ? color * texture(diffuseTextures[textureIndex-1], tc) : color; 
    vec3 e1 = v2 - v1;
    vec3 e2 = v3 - v1;
    vec3 N = normalize(cross(e1, e2));
    vec3 rayDir = gl_WorldRayDirectionEXT;
    vec3 hitPos = gl_WorldRayOriginEXT + rayDir * gl_HitTEXT;
    if (dot(N, rayDir) > 0.0) {
        N = -N;
    }
    bool entering = dot (rayDir, N) < 0;
    vec3 I = normalize(-gl_WorldRayDirectionEXT);
    //Seul les rayons primaires et secondaires mettent à jour le transport pour le rayon suivant. 
    if (transport.rayType == 0 || transport.rayType == 6) {
        transport.normal = N;
        transport.parentMaterial = material;
        transport.localASID = int(geomOffs.tlasID);
        transport.origin = hitPos + N * epsilon;
        transport.direction = rayDir; 
        if(material.reflectable == 1) {
            transport.R = reflect(I, N);
            transport.reflectable = true;          
        } else {
            transport.reflectable = false;
        }   
        if (material.refractable == 1) {
            float IOR = 1;
            if (material.materialType == 1) {
                IOR = 1.00 / 1.33;        
            } else if (material.materialType == 2) {
                IOR = 1.00 / 1.309;        
            } else if (material.materialType == 3) {
                IOR = 1.00 / 1.52;        
            } else if (material.materialType == 4) {
                IOR = 1.00 / 2.42;        
            }
            float eta = entering ? (1.0 / IOR) : IOR;
            transport.T = refract(I, N, eta);
            transport.refractable = true;        
        } else {
            transport.refractable = false;
        } 
        if (material.opaque == 0) {
            transport.transmissive  = true;
        } else {
            transport.transmissive = false;
        }
    } 
    //Calcul de la couleur du rayon courant.  
    //Rayon primaire. (Je stocke la couleur secondaire)     
    if (transport.rayType == 0) {
        transport.transmitionColor = albedo;      
    //Rayon de transmission.
    } else if (transport.rayType == 1) {
        vec3 absorption = exp(-albedo.xyz * 1);
        transport.transmitionColor *= vec4(absorption, 1);
        //Rayon de réflection.
    } else if (transport.rayType == 2) {
        transport.reflectColor = albedo;        
        //Rayon de réfractrion.    
    } else if (transport.rayType == 3) {
        transport.refractColor = albedo;        
    //Shadow ray. (Lumière ponctuelles et directionnelles)
    } else if (transport.rayType > 3) {        
        transport.shadowColor = (material.opaque == 0) ? vec4(0.5, 0.5, 0.5, 1) : albedo;
        transport.localLightning = transport.shadowColor * transport.lightColor * vec4(N, 1);//Calcul de la couleur de la lumière locale. (Ombre partielle)
    } 
    /*if (payload.color.r > 1 || payload.color.g > 1 || payload.color.b > 1 || payload.color.a > 1
    || payload.color.r < 0 || payload.color.g < 0 || payload.color.b < 0 || payload.color.a < 0) {*/
        //debugPrintfEXT("color : %v4f", payload.color);
    /*}*/
}