#version 460
#extension GL_EXT_multiview : enable
layout (location = 0) in vec3 aPos;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 normals;
layout(location = 4) in uint drawableDataId;
layout (location = 0) out vec3 worldPos;

layout (push_constant) uniform PushConstant {
    mat4 projection;   
} pc;
layout(std140, set = 0, binding = 0) uniform ViewsUBO {
    mat4 views[6];
} viewsData;
void main()
{
    worldPos = aPos;  
    gl_Position =  pc.projection * viewsData.views[gl_ViewIndex] * vec4(worldPos, 1.0);
}