#version 460
layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 normals;
layout (location = 4) in uint drawableDataID;
void main () {
     gl_PointSize = 2.0f;
     gl_Position = vec4(position, 1.f);
};
