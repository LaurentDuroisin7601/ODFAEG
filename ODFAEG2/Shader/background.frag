#version 460 core
layout (location = 0) out vec4 fragColor;
layout (location = 0) in vec3 worldPos;

layout (set = 0, binding = 0) uniform samplerCube environmentMap;

void main()
{		
    vec3 envColor = textureLod(environmentMap, worldPos, 0.0).rgb;
    
    // HDR tonemap and gamma correct
    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0/2.2)); 
    
    fragColor = vec4(envColor, 1.0);
}
