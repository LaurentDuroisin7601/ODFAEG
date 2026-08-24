#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_debug_printf : enable
#define MAX_FRAMES_IN_FLIGHT 2
#define MAX_PRIMITIVE_TYPES 6
#define MAX_TEXTURES 1024
#define MAX_BONES_INFLUENCE 4
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
layout (binding = 8, set = 1) uniform sampler2D diffuseTextures[MAX_TEXTURES];
struct TransportRayPayload {
    bool lastBounce;
    int raytype;
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
    int localADID;  
};
struct ShadowRayPayload {
    vec4 localLightning;
    vec3 lightPos;
    int lightId;        
    vec4 lightColor; 
    vec4 shadowColor; 
    mat4 lightSpace[6];
};
layout(location = 0) rayPayloadInEXT TransportPayload transport;
layout(location = 1) rayPayloadInEXT ShadowPayload shadow;
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
    vec3 v1 = verticesData[3].vertices[geomOffs.vertexOffset + i1].position;
    vec3 v2 = verticesData[3].vertices[geomOffs.vertexOffset + i2].position;
    vec3 v3 = verticesData[3].vertices[geomOffs.vertexOffset + i3].position;
    float u = baryCoords.x;
    float v = baryCoords.y;
    float w = 1.0 - u - v;
    vec4 color = w * c1 + u * c2 + v * c3;
    vec2 tc = w * ct1 + u * ct2 + v * ct3;
    uint textureIndex = material.diffuseTextureIndex;  
    vec4 aldebo = (textureIndex > 0) ? color * texture(diffuseTextures[textureIndex-1], tc) : color; 
    vec3 e1 = v2 - v1;
    vec3 e2 = v3 - v1;
    vec3 N = e1.cross(e2).normalize();
    vec3 rayDir = gl_WorldRayDirectionEXT;
    vec3 hitPos = gl_WorldRayOriginEXT + rayDir * gl_HitTEXT;
    if (dot(N, rayDir) > 0.0) {
        N = -N;
    }
    bool entering = dot (rayDir, n) < 0;
    vec3 I = normalize(-gl_WorldRayDirectionEXT);
    //Si ce n'est pas un shadow ray, on met à jour le transport du rayon pour le rayon suivant. 
    if (transport.raytype < 4) {
        transport.localASID = geomOffs.tlasID;
        transport.origin = hitpos + N * espilon;
        transport.direction = raydir;
        if (material.opaque == 1) {
            transport.transmition  = false;
        } else {
            transport.transmition  = true;
        }
        if(material.reflectable == 1) {
            payload.R = refract(I, N);
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
    }  
    //Calcul de la couleur du rayon courant.  
    //Rayon primaire. (Je stocke la couleur secondaire)     
    if (transport.rayType == 0) {
        transport.transmitionColor = albedo;      
    //Rayon secondaire non dévié.
    } else if (transposrt.raytype == 1) {
        vec3 absorption = exp(-aldebo * 1);
        transport.transmissionColor *= absorption;
        //Rayon de réflection.
    } else if (transport.rayType == 2) {
        transport.reflectColor = albedo;        
        //Rayon de réfractrion.    
    } else if (transport.rayType == 3) {
        transport.refractColor = albedo;        
    //Shadow ray. (Lumière ponctuelles et directionnelles)
    } else if (transposrt.raytype > 3) {        
        shadow.shadowColor = (material.opaque) ? vec4(0.5, 0.5, 0.5, 1) : albedo;
        shadow.localLighting = shadow.shadowColor * N * shadowpayload.lightColor * N;//Calcul de la couleur de la lumière locale. (Ombre partielle)
    } 
    /*if (payload.color.r > 1 || payload.color.g > 1 || payload.color.b > 1 || payload.color.a > 1
    || payload.color.r < 0 || payload.color.g < 0 || payload.color.b < 0 || payload.color.a < 0) {*/
        //debugPrintfEXT("color : %v4f", payload.color);
    /*}*/
}