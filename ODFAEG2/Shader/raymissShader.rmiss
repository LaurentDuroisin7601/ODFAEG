#version 460
#extension GL_EXT_ray_tracing : enable
#define NB_CASCADES 4
#define MAX_FRAMES_IN_FLIGHT 2
#define MAX_TEXTURES 1024
struct RayPayload {
    vec4 color;
    bool hit;
};
layout(binding = 0, set = 0) uniform CameraProperties {
    mat4 viewInverse;
    mat4 projInverse;    
} cam[MAX_FRAMES_IN_FLIGHT];
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
    int localADID; 
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
layout (push_constant) uniform PushConstant {
    int currentFrame;
} pc;
layout (binding = 4, set = 1) uniform samplerCube skybox;
layout (binding = 5, set = 1) uniform sampler2DArray csmShadowMaps;
layout (binding = 6, set = 1) uniform samplerCubeArray pointShadowMaps; 
layout (binding = 7, set = 1) uniform sampler2D frameBuffer;
layout(set = 2, binding = 0) uniform sampler2D specularTextures[MAX_TEXTURES];
layout(set = 3, binding = 0) uniform sampler2D normalTextures[MAX_TEXTURES];
layout(set = 4, binding = 0) uniform sampler2D metalnessTextures[MAX_TEXTURES];
layout(set = 5, binding = 0) uniform sampler2D roughnessTextures[MAX_TEXTURES];
layout(set = 6, binding = 0) uniform sampler2D aoTextures[MAX_TEXTURES];
layout(set = 7, binding = 0) uniform sampler2D emissiveTextures[MAX_TEXTURES];
layout(location = 0) rayPayloadInEXT TransportRayPayload transport;
void main()
{        
    //Rayon primaire.
    if (transport.rayType < 2) {
       vec4 proj = inverse(cam[pc.currentFrame].viewInverse) * inverse(cam[pc.currentFrame].projInverse) * vec4(transport.origin, 1);
       proj = proj.xyz / proj.w; 
       transport.primaryColor = texture(framebuffer, vec2(proj.xy)); 
       transport.transmission = false;
       transport.reflectable = false;
       transport.refractable = false;
    } else if (transport.rayType == 2) {
       transport.reflectColor = texture(skybox, transport.R);
       transport.reflectable = false;
    } else if (transport.rayType == 3) {
       transport.reflectColor = texture(skybox, transport.T);
       transport.refractable = false;
    } else if (transport.rayType == 4) {
       vec3 rayDir = gl_WorldRayDirectionEXT; 
       vec3 hitPos = gl_WorldRayOriginEXT + rayDir * gl_HitTEXT;
       vec4 proj = transport.dirLightSpace[0] * hitPos;
       proj.xyz = proj.xyz / w;
       float depth = texture(cmsShadowMaps, vec3(proj.xy, transport.lightId));
       bool inShadow =  (proj.z < depth);
       transport.localLighthing = (inShadow) ? vec4(0.5, 0.5, 0.5, 1) * transport.lightColor : shadow.lightColor;
    } else if (transport.rayType == 5) {
       vec3 rayDir = gl_WorldRayDirectionEXT; 
       vec3 hitPos = gl_WorldRayOriginEXT + rayDir * gl_HitTEXT;
       vec3 fragToLight = (transport.lightPos - hitPos);
       vec3 ad = abs(fragToLight);
       int face; 
       vec3 d =  fragToLight;      
       if (ad.x >= ad.y && ad.x >= ad.z)
           face = d.x > 0 ? 0 : 1; // +X / -X
       else if (ad.y >= ad.x && ad.y >= ad.z)
           face = d.y > 0 ? 2 : 3; // +Y / -Y
       else
           face = d.z > 0 ? 4 : 5; // +Z / -Z
       vec2 uv;
       vec4 proj = transport.pointLightSpace[face] * hitPos;
       proj.xyz = proj.xyz / w;
       float depth = texture(cmsShadowMaps, vec4(vec3(proj.xy, face), float(transport.lightId)));
       bool inShadow = (proj.z < depth);
       transport.localLighthing = (inShadow) ? vec4(0.5, 0.5, 0.5, 1) * transport.lightColor * normal : transport.lightColor * normal;
    }
}