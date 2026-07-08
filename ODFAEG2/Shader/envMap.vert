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

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 normal;
layout(location = 3) flat out int v_DrawID;
layout(location = 4) flat out int primitiveType;
layout(location = 5) flat out int currentFrame;
layout (push_constant) uniform PushConstant {
    mat4 projMatrix;    
    int primitiveType;
    int currentFrame;
} pc;
layout (std430, set = 0, binding = 0) buffer ModelDataSSBO {
    ModelData modelData[];
} modelDataBuffer[NB_PRIMITIVE_TYPES * MAX_FRAMES_IN_FLIGHT];
layout (std140, set = 0, binding = 1) uniform ViewMatricesUBO {
    mat4 viewMatrices[6];
} viewMatricesDatas[MAX_FRAMES_IN_FLIGHT];
void main() {
    //debugPrintfEXT("Vertex shader ok!");
    gl_PointSize = 2.0f;
    mat4 modelMatrix = modelDataBuffer[pc.primitiveType*MAX_FRAMES_IN_FLIGHT+pc.currentFrame].modelData[gl_InstanceIndex].modelMatrix;
    gl_Position =  pc.projMatrix * viewMatricesDatas[pc.currentFrame].viewMatrices[gl_ViewIndex] * modelMatrix * vec4(inPosition, 1);
    //debugPrintfEXT("Proj matrix : 0:%v4f\n1:%v4f\n2:%v4f\n3:%v4f",pc.projMatrix[0], pc.projMatrix[1], pc.projMatrix[2], pc.projMatrix[3]);
    //debugPrintfEXT("View matrix : 0:%v4f\n1:%v4f\n2:%v4f\n3:%v4f",pc.viewMatrix[0], pc.viewMatrix[1], pc.viewMatrix[2], pc.viewMatrix[3]);
    //debugPrintfEXT("Model matrix : 0:%v4f\n1:%v4f\n2:%v4f\n3:%v4f",modelMatrix[0], modelMatrix[1], modelMatrix[2], modelMatrix[3]);
    //debugPrintfEXT("in position %v3f, out position %v4f", inPosition, gl_Position);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    normal = normals;
    v_DrawID = gl_DrawID;
    primitiveType = pc.primitiveType;
    currentFrame = pc.currentFrame;    
}
