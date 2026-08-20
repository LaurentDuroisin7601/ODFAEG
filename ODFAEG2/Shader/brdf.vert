#version 460
layout (location = 0) in vec3 aPos;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 normals;
layout(location = 4) in uint drawableDataId;
layout (location = 0) out vec2 texCoords;
void main()
{
    texCoords = aTexCoords;
	gl_Position = vec4(aPos, 1.0);
}