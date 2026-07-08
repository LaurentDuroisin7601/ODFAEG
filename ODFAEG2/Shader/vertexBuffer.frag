#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_debug_printf : enable
const uint MAX_TEXTURES = 1024;
layout(set = 0, binding = 1) uniform sampler2D textures[MAX_TEXTURES];
layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in flat uint inTextureID;
layout(location = 0) out vec4 outColor;
void main() {
	outColor = (inTextureID == 0) ? fragColor : texture(textures[inTextureID-1], fragTexCoord) * fragColor;
}
