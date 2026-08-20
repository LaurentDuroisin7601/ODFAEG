#version 460
layout (location = 0) in vec3 aPos;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 normals;
layout(location = 4) in uint drawableDataId;
layout (push_constant) uniform PushConstant {
    mat4 projection;
    mat4 view;
} pc;
layout (location = 0) out vec3 worldPos;
void main()
{
    worldPos = aPos;
	mat4 rotView = mat4(mat3(pc.view));
	vec4 clipPos = pc.projection * rotView * vec4(worldPos, 1.0);

	gl_Position = clipPos.xyww;
}