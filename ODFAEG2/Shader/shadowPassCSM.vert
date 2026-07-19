#version 460
#extension GL_EXT_debug_printf : enable
#extension GL_EXT_multiview : enable
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
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 normals;
layout(location = 4) in uint drawableDataId;
layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 normal;
layout (location = 3) out flat int v_DrawID;
layout (location = 4) out flat int primitiveType;
layout (location = 5) out flat int currentFrame;
layout (push_constant) uniform PushConstant {
    int primitiveType;    
    int currentFrame;
} pc;
layout (std430, set = 0, binding = 0) buffer ModelDataSSBO {
    ModelData modelData[];
} modelDataBuffer[NB_PRIMITIVE_TYPES * MAX_FRAMES_IN_FLIGHT];
layout (std140, set = 0, binding = 2) buffer LightMatricesSSBO {
    LightSpaceMatrix lightSpaceMat;   
} lightMatsData[MAX_FRAMES_IN_FLIGHT];
void main() {
    //debugPrintfEXT("shader");
    gl_PointSize = 2.0f;
    mat4 modelMatrix = modelDataBuffer[pc.primitiveType*MAX_FRAMES_IN_FLIGHT+pc.currentFrame].modelData[gl_InstanceIndex].modelMatrix;
    gl_Position = lightMatsData[pc.currentFrame].lightSpaceMat.lightSpaceMatrices[gl_ViewIndex] * modelMatrix * vec4(inPosition, 1);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    normal = transpose(inverse(mat3(modelMatrix))) * normals;
    v_DrawID = gl_DrawID;
    primitiveType = pc.primitiveType;
    currentFrame = pc.currentFrame;
    /*if (gl_ViewIndex == 1)
    debugPrintfEXT("view index : %i",gl_ViewIndex);*/
    //debugPrintfEXT("model matrix : %v4f\n%v4f\n%v4f\n%v4f\nposition : %v4f", modelMatrix[0],modelMatrix[1],modelMatrix[2],modelMatrix[3], gl_Position);
    //debugPrintfEXT("light space matrix : %v4f\n%v4f\n%v4f\n%v4f\nposition : %v4f", lightMatsData[pc.currentFrame].lightSpaceMats[gl_ViewIndex][0],lightMatsData[pc.currentFrame].lightSpaceMats[gl_ViewIndex][1],lightMatsData[pc.currentFrame].lightSpaceMats[gl_ViewIndex][2],lightMatsData[pc.currentFrame].lightSpaceMats[gl_ViewIndex][3], gl_Position);
    //debugPrintfEXT("position : %v4f", gl_Position);
}
