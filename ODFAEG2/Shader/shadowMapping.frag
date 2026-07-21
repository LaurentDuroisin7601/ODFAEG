#version 460
#extension GL_EXT_debug_printf : enable
#define NB_CASCADES 4
#define MAX_TEXTURES 1024
#define NB_PRIMITIVE_TYPES 6
#define MAX_FRAMES_IN_FLIGHT 2
#define NB_SWAPCHAIN_IMAGES 3
layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) flat in int v_DrawID;
layout(location = 4) flat in int primitiveType;
layout(location = 5) flat in int currentFrame;
layout(location = 6) in vec3 fragPos;
layout(location = 0) out vec4 frag_color;
#define NB_CASCADES 4
struct NodeType {
    vec4 color;
    float depth;
    uint next;
};
struct LightSpaceMatrix {
    mat4 lightSpaceMatrices[NB_CASCADES+1];
};
struct DirLight {    
    vec3 lightDir;
    float far_plane;
};
struct PointLight {
    vec3 lightPos;
    float far_plane;
};
layout (push_constant) uniform PushConstant { 
    layout(offset=144) uint view;     
    layout(offset=208) uint nbDirLights;
    layout(offset=212) uint nbPointLights; 
    layout(offset=216) uint imageIndex;  
    layout(offset=224) ivec2 resolution;  
} pc;
layout (std430, set = 0, binding = 1) buffer LightSpaceMatricesSSBO {
    LightSpaceMatrix lightSpaceMatrices[];
} lightSpaceMatricesData[MAX_FRAMES_IN_FLIGHT];
layout (std140, set = 0, binding = 2) uniform cascadePlaneDistanceSSBO {
    float cascadePlaneDistances[NB_CASCADES+1];
    int cascadeCount;   // number of
} cascadePlaneDistanceData[MAX_FRAMES_IN_FLIGHT];
layout(set = 0, binding = 3) buffer DirLightSSBO {
    DirLight dirLights[];
} dirLightData[MAX_FRAMES_IN_FLIGHT];
layout(set = 0, binding = 4) buffer PointLightSSBO {
    PointLight pointLights[];
} pointLightData[MAX_FRAMES_IN_FLIGHT];
layout(set = 0, binding = 5) uniform sampler2DArray shadowMap;
layout(set = 0, binding = 6) uniform samplerCube depthMap;
layout(set = 0, binding = 7) uniform sampler2D sceneColorTextures;
layout(set = 0, binding = 8, r32ui) uniform coherent uimage2D headPointersDir[MAX_FRAMES_IN_FLIGHT*(NB_CASCADES+1)];
layout(set = 0, binding = 9) buffer LinkedListDirBufferSSBO {
    NodeType nodes[];
} linkedListDirData[MAX_FRAMES_IN_FLIGHT*(NB_CASCADES+1)];
layout(set = 0, binding = 10, r32ui) uniform coherent uimage2D headPointersPoint[MAX_FRAMES_IN_FLIGHT*6];
layout(set = 0, binding = 11) buffer LinkedListPointBufferSSBO {
    NodeType nodes[];
} linkedListPointData[MAX_FRAMES_IN_FLIGHT*6];
float shadowCalculationDir(vec3 fragPosWorldSpace)
{
    // select cascade layer 
    float shadow = 0.0;  
    for(int l = 0; l < pc.nbDirLights; l++) {
        vec4 fragPosViewSpace = pc.view * vec4(fragPosWorldSpace, 1.0);    
        float depthValue = abs(fragPosViewSpace.z);
        int layer = -1; 
        for (int i = 0; i < NB_CASCADES; ++i)
        {       
            if (depthValue < cascadePlaneDistanceData[currentFrame].cascadePlaneDistances[i])
            {
                
                layer = i;
                break;
            }
        }
        if (layer == -1)
        {
            layer = NB_CASCADES;
        } 
        vec4 fragPosLightSpace =  lightSpaceMatricesData[currentFrame].lightSpaceMatrices[l].lightSpaceMatrices[layer] * vec4(fragPosWorldSpace, 1.0);
        vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
        projCoords.xy = projCoords.xy * 0.5 + 0.5;
        //debugPrintfEXT("Proj coords : %v4f", projCoords);
        // get depth of current fragment from light's perspective
        float currentDepth = projCoords.z;
        // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
        if (currentDepth < 0 || currentDepth > 1.0)
        {            
            return 0.0;
        }
        // calculate bias (based on depth map resolution and slope)
        vec3 normal = normalize(normal);
        //debugPrintfEXT("light dir : %f", dirLightData[currentFrame].dirLights[l].far_plane);
        float bias = max(0.05 * (1.0 - dot(normal,dirLightData[currentFrame].dirLights[l].lightDir)), 0.005);
        const float biasModifier = 0.5f;
        if (layer == NB_CASCADES)
        {
            bias *= 1 / (dirLightData[currentFrame].dirLights[l].far_plane * biasModifier);
        }
        else
        {
            bias *= 1 / (cascadePlaneDistanceData[currentFrame].cascadePlaneDistances[layer] * biasModifier);
        }
        // PCF
        float currentLightShadow = 0.0;
        vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));        
        for(int x = -1; x <= 1; ++x)
        {
            for(int y = -1; y <= 1; ++y)
            {
                float pcfDepth = texture(shadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;               
                currentLightShadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
            }
        }
        shadow = max(currentLightShadow, shadow);
    }     
    shadow /= 9.0;
    return shadow;
}
float shadowCalculationPoint(vec3 fragPos)
{
    // get vector between fragment position and light position
    // get vector between fragment position and light position
    float shadow = 0.0;
    for(int l = 0; l < pc.nbPointLights; l++) {
        //debugPrintfEXT("nb lights : %i", pc.nbPointLights);
        vec3 fragToLight = fragPos - pointLightData[currentFrame].pointLights[l].lightPos;
        //debugPrintfEXT("Frag to light %v3f", fragToLight);
        // use the light to fragment vector to sample from the depth map    
        float closestDepth = texture(depthMap, fragToLight).r;
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
        if (face == 0) {
            uv = vec2(-d.z, -d.y) / abs(d.x);
        } else if (face == 1) {
            uv = vec2(d.z, -d.y) / abs(d.x);
        } else if (face == 2) {
            uv = vec2(d.x, d.z) / abs(d.y);
        } else if (face == 3) {
            uv = vec2(d.x, -d.z) / abs(d.y);
        } else if (face == 4) {
            uv = vec2(d.x, -d.y) / abs(d.z);
        } else {
            uv = vec2(-d.x, -d.y) / abs(d.z);
        }
        uv = uv * 0.5 + 0.5;
        //debugPrintfEXT("cosest depth : %f",closestDepth);
        // it is currently in linear range between [0,1]. Re-transform back to original value
        closestDepth *= pointLightData[currentFrame].pointLights[l].far_plane;
        // now get current linear depth as the length between the fragment and light position
        float currentDepth = length(fragToLight);
        // now test for shadows
        float bias = 0.05; 
        vec3 lightDir = normalize(pointLightData[currentFrame].pointLights[l].lightPos - fragPos);
        //debugPrintfEXT("lightpos: %f", pointLightData[currentFrame].pointLights[l].far_plane);
        float diff = max(dot(lightDir, normalize(normal)), 0.0); 
        
        /*if (currentDepth -  bias <= closestDepth)
            debugPrintfEXT("not in shadow"); */  
        float currentLightShadow = currentDepth -  bias > closestDepth ? min(1.0, 1.0 * diff) : 0.0; 
        shadow = max(currentLightShadow, shadow);  
    } 
    return shadow;
}  
void main()
{    
    //debugPrintfEXT("draw");    
    vec2 uv = gl_FragCoord.xy / pc.resolution;
    //debugPrintfEXT("uv %v2f, resolution %v2i", uv, pc.resolution);
    vec4 sceneColor = texture(sceneColorTextures, uv);
    /*if (sceneColor.r != 0 || sceneColor.r != 0 || sceneColor.b != 0)
        debugPrintfEXT("scene color %v4f", sceneColor);*/
    vec3 normal = normalize(normal);
    // calculate shadow
    float shadowDir = shadowCalculationDir(fragPos);
    float shadowPoint = shadowCalculationPoint(fragPos);
    /*if (shadowPoint > 0)
        debugPrintfEXT("Shadow point : %f", shadowPoint);*/
    float shadow = max(shadowDir, shadowPoint);
    /*if (shadowDir > 0 || shadowPoint > 0)
        debugPrintfEXT("shadow dir %f", shadow);*/   
    frag_color = vec4(vec3(sceneColor) * (1 - shadow), sceneColor.a);
}
