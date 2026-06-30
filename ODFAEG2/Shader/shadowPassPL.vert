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
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 normals;
layout(location = 4) in uint drawableDataId;
layout(location = 0) out vec4 fragPos;
layout (push_constant) uniform PushConstant {
    mat4 lightProjMatrix;
    int primitiveType;    
    int currentFrame;    
} pc;
layout (std430, set = 0, binding = 0) buffer ModelDataSSBO {
    ModelData modelData[];
} modelDataBuffer[NB_PRIMITIVE_TYPES * MAX_FRAMES_IN_FLIGHT];
layout (std430, set = 0; binding = 1) uniform LightViewMatrices {
    mat4 lightViewMatrices[6];
} lightViewMatricesData[MAX_FRAMES_IN_FLIGHT];
void main() {
    //debugPrintfEXT("shader");
    gl_PointSize = 2.0f;
     mat4 modelMatrix = modelDataBuffer[pc.primitiveType*MAX_FRAMES_IN_FLIGHT+pc.currentFrame].modelData[gl_InstanceIndex].modelMatrix;
    fragPos = modelMatrix * vec4(inPosition, 1);   
    gl_Position = lightProjMatrix * lightViewMatricesData[pc.currentFrame].lightViewMatrices[gl_ViewIndex] * fragPos;    
}