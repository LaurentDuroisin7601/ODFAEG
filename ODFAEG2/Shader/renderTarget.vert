#version 460
#extension GL_EXT_debug_printf : enable
#define NB_PRIMITIVE_TYPES 6
#define MAX_FRAMES_IN_FLIGHT 2
struct ModelData {
    mat4 modelMatrix;
    mat4 shadowProjMatrix;
    mat4 borderMatrices;
};
struct AABB {
	vec3 center;
	vec3 size;
};
struct SubMesh {
	AABB globalBounds;
	int vertexOffset;
	int indexOffset;
	int primitiveType;
	int materialId;
	int nbVertices;
	int nbIndexes;
	int lodOffset;
	int id;
    int lodLevel;
    int objectId;
    int mesheltOffset;
    int meshletCount;
};
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 normals;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBitangent;
layout(location = 6) in vec3 inNormal;
layout(location = 7) in uint drawableDataId;


layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 normal;
layout(location = 3) flat out int materialID;
layout(location = 4) flat out int primitiveType;
layout(location = 5) flat out int currentFrame;
layout(location = 6) out vec3 vTangent;
layout(location = 7) out vec3 vBitangent;
layout(location = 8) out vec3 vNormal;
layout(location = 9) flat out int meshId;
layout (push_constant) uniform PushConstant {
    mat4 projMatrix;
    mat4 viewMatrix;
    int primitiveType;
    int currentFrame;
} pc;
layout (std430, set = 0, binding = 0) buffer ModelDataSSBO {
    ModelData modelData[];
} modelDataBuffer[NB_PRIMITIVE_TYPES * MAX_FRAMES_IN_FLIGHT];
layout (std430, set = 0, binding = 1) buffer SubMeshDataSSBO {
    SubMesh subMesh[];
} subMeshData[NB_PRIMITIVE_TYPES * MAX_FRAMES_IN_FLIGHT];
void main() {
    //debugPrintfEXT("primitive type : %i, current frame : %i", pc.primitiveType, pc.currentFrame);
    gl_PointSize = 2.0f;
    SubMesh subMesh = subMeshData[pc.primitiveType*MAX_FRAMES_IN_FLIGHT+pc.currentFrame].subMesh[gl_DrawID];
    meshId = subMesh.id;
    mat4 modelMatrix = modelDataBuffer[pc.primitiveType*MAX_FRAMES_IN_FLIGHT+pc.currentFrame].modelData[gl_InstanceIndex].modelMatrix;
    gl_Position =  pc.projMatrix * pc.viewMatrix * modelMatrix * vec4(inPosition, 1);
    if (gl_Position.x < -gl_Position.w || 
        gl_Position.x > gl_Position.w ||
        gl_Position.y < -gl_Position.w || 
        gl_Position.y > gl_Position.w ||
        gl_Position.z < 0 ||
        gl_Position.z > gl_Position.w) {
            debugPrintfEXT("mesh : %i out of frustrum pos : %v3f", meshId, inPosition);
        }
    //debugPrintfEXT("Proj matrix : 0:%v4f\n1:%v4f\n2:%v4f\n3:%v4f",pc.projMatrix[0], pc.projMatrix[1], pc.projMatrix[2], pc.projMatrix[3]);
    //debugPrintfEXT("View matrix : 0:%v4f\n1:%v4f\n2:%v4f\n3:%v4f",pc.viewMatrix[0], pc.viewMatrix[1], pc.viewMatrix[2], pc.viewMatrix[3]);
    //debugPrintfEXT("Model matrix : 0:%v4f\n1:%v4f\n2:%v4f\n3:%v4f",modelMatrix[0], modelMatrix[1], modelMatrix[2], modelMatrix[3]);
    //debugPrintfEXT("in position %v3f, out position %v4f", inPosition, gl_Position);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    normal = mat3(transpose(inverse(modelMatrix))) * normals;
    materialID = /*subMesh.materialId*/gl_DrawID;
    primitiveType = pc.primitiveType;
    currentFrame = pc.currentFrame;
    vTangent = inTangent;
    vBitangent = inBitangent;
    vNormal = inNormal;
    //debugPrintfEXT("tangent : %v3f, bitangeant : %v3f, normal : %v3f", vTangent, vBitangent, vNormal);
}
