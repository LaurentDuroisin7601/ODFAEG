#version 460
#extension GL_EXT_ray_tracing : enable
struct RayPayload {
    vec4 color;
    bool hit;
};
layout(location = 0) rayPayloadInEXT RayPayload payload;
void main()
{
    if (!payload.hit) {
        payload.color = vec4(0, 0, 1, 1); // bleu seulement si aucun hit
    }
}