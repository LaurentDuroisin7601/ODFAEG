#version 460
#extension GL_EXT_debug_printf : enable
#define NB_PRIMITIVE_TYPES 6
#define MAX_FRAMES_IN_FLIGHT 2
#define NB_CASCADES 4
struct ModelData {
    mat4 modelMatrix;
    mat4 shadowProjMatrix;
    mat4 borderMatrices;
};
struct LightSpaceMatrix {
    mat4 lightSpaceMatrices[NB_CASCADES+1];
};
struct ViewPLMatrix {
    mat4 viewPLMatrices[6];
};
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 normals;
layout(location = 4) in uint drawableDataId;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 normal;
layout(location = 3) flat out int v_DrawID;
layout(location = 4) flat out int primitiveType;
layout(location = 5) flat out int currentFrame;
layout(location = 6) out vec3 fragPos;
layout(location = 7) out vec4 clipPos; 
layout (push_constant) uniform PushConstant {
    mat4 projMatrix;
    mat4 viewMatrix;
    int primitiveType;
    int currentFrame;
    int pad[2];
} pc;
layout (std430, set = 0, binding = 0) buffer ModelDataSSBO {
    ModelData modelData[];
} modelDataBuffer[NB_PRIMITIVE_TYPES * MAX_FRAMES_IN_FLIGHT];
void main() {
    //debugPrintfEXT("Vertex shader ok!");
    gl_PointSize = 2.0f;
    mat4 modelMatrix = modelDataBuffer[pc.primitiveType*MAX_FRAMES_IN_FLIGHT+pc.currentFrame].modelData[gl_InstanceIndex].modelMatrix;
    gl_Position =  pc.projMatrix * pc.viewMatrix * modelMatrix * vec4(inPosition, 1); 
    clipPos = pc.projMatrix * pc.viewMatrix * modelMatrix * vec4(inPosition, 1); 
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    normal = transpose(inverse(mat3(modelMatrix))) * normals;
    v_DrawID = gl_DrawID;
    primitiveType = pc.primitiveType;
    currentFrame = pc.currentFrame;
    fragPos = vec3(modelMatrix * vec4(inPosition, 1));    
}
