#version 460
#extension GL_EXT_debug_printf : enable
#define MAX_FRAMES_IN_FLIGHT 2
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 normals;
layout(location = 4) in uint drawableDataId;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 normal;
layout(location = 3) out uint outTextureID;

layout (push_constant) uniform PushConstant {
    mat4 projMatrix;
    mat4 viewMatrix;
    mat4 modelMatrix;
    vec2 uvScale;
    vec2 uvOffset;
    int nbBuffers;
    int textureIndex;
    int currentFrame;
    int currentImageIndex;
} pc;
struct VertexBufferData {
	mat4 projMatrix;
	mat4 viewMatrix;
    mat4 modelMatrix;
    vec2 uvScale;
    vec2 uvOffset;
    int nbBuffers;
    int textureIndex;
};
layout (std430, set = 0, binding = 0) buffer VertexBufferDataSSBO {
     VertexBufferData vertexBufferDatas[];
} vertexBuffer;
void main() {

        gl_PointSize = 2.0f;
        gl_Position =  transpose(pc.projMatrix) * transpose(pc.viewMatrix) * transpose(pc.modelMatrix) * vec4(inPosition, 1);
        // debugPrintfEXT("position : %v4f", gl_Position);
        fragColor = inColor;
        fragTexCoord = inTexCoord /** pc.uvScale + pc.uvOffset*/;
        normal = mat3(transpose(inverse(pc.modelMatrix))) * normals;
        outTextureID = (pc.textureIndex==0) ? 0 : pc.textureIndex*pc.nbBuffers+min(pc.currentImageIndex, pc.nbBuffers-1);
}
