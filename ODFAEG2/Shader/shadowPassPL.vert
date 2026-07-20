#version 460
#extension GL_EXT_debug_printf : enable
#extension GL_EXT_multiview : enable
#define NB_PRIMITIVE_TYPES 6
#define MAX_FRAMES_IN_FLIGHT 2
struct ModelData {
    mat4 modelMatrix;
    mat4 shadowProjMatrix;
    mat4 borderMatrices;
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
layout(location = 3) out vec4 fragPos;
layout(location = 4) out flat int v_DrawID;
layout(location = 5) out flat int primitiveType;
layout(location = 6) out flat int currentFrame;
layout (push_constant) uniform PushConstant {
    mat4 lightProjMatrix;
    int primitiveType;    
    int currentFrame;    
} pc;
layout (std430, set = 0, binding = 0) buffer ModelDataSSBO {
    ModelData modelData[];
} modelDataBuffer[NB_PRIMITIVE_TYPES * MAX_FRAMES_IN_FLIGHT];
layout (std140, set = 0, binding = 1) buffer LightViewMatrices {
    ViewPLMatrix viewPLMatrix;
} lightViewMatricesData[MAX_FRAMES_IN_FLIGHT];
void main() {
    //debugPrintfEXT("shader");
    gl_PointSize = 2.0f;
    mat4 modelMatrix = modelDataBuffer[pc.primitiveType*MAX_FRAMES_IN_FLIGHT+pc.currentFrame].modelData[gl_InstanceIndex].modelMatrix;
    fragPos = modelMatrix * vec4(inPosition, 1);  
    //debugPrintfEXT("viewPLMatrix : %v4f", lightViewMatricesData[pc.currentFrame].viewPLMatrix.viewPLMatrices[gl_ViewIndex][0]); 
    gl_Position = pc.lightProjMatrix * lightViewMatricesData[pc.currentFrame].viewPLMatrix.viewPLMatrices[gl_ViewIndex] * fragPos;  
    
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    normal = transpose(inverse(mat3(modelMatrix))) * normals;
    v_DrawID = gl_DrawID;
    primitiveType = pc.primitiveType;
    currentFrame = pc.currentFrame;
}