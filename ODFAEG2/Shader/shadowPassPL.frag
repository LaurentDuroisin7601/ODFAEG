#version 460 core
layout(location=0) in vec4 fragPos;
layout (push_constant) uniform PushConstant {
    layout(offset=80) vec3 lightPos;    
    layout(offset=96) float far_plane;
} pc;
void main()
{
    // get distance between fragment and light source
    float lightDistance = length(fragPos.xyz - pc.lightPos);    
    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / pc.far_plane;    
    // write this as modified depth
    gl_FragDepth = lightDistance;
}  