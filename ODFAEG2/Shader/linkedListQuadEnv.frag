#version 460
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_debug_printf : enable
#extension GL_EXT_multiview : enable
#define MAX_FRAMES_IN_FLIGHT 2
#define MAX_FRAGMENTS 20
#define PPLL_RESOLUTION 512
struct NodeType {
  vec4 color;
  float depth;
  uint next;
  uint hResId; 
};
layout (push_constant) uniform PushConstant {
  vec2 resolution;
  int currentFrame;
} pc;
layout(set = 0, binding = 0, r32ui) uniform uimage2D headPointers[MAX_FRAMES_IN_FLIGHT*6];
layout(std430, set = 0, binding = 1) buffer linkedLists {
   NodeType nodes[];
} nodeData[MAX_FRAMES_IN_FLIGHT*6];
layout(location = 0) out vec4 fcolor;
void main() {
  //debugPrintfEXT("Full quad fs");
  NodeType frags[MAX_FRAGMENTS];
  int count = 0;
  ivec2 hrLrScale = ivec2(pc.resolution.xy) / PPLL_RESOLUTION;
  ivec2 lrFragCoord = ivec2(gl_FragCoord.xy) / hrLrScale; 
  uint n = imageLoad(headPointers[pc.currentFrame], lrFragCoord).r;  
  uint hResId = uint((int(gl_FragCoord.x) % int(hrLrScale.x)) + (int(gl_FragCoord.y) % int(hrLrScale.y)) * hrLrScale.x);
  /*if (nodeData[pc.currentFrame].nodes[n].hResId > 0 && nodeData[pc.currentFrame].nodes[n].hResId < 3) {
    debugPrintfEXT("h res id : %i", nodeData[pc.currentFrame].nodes[n].hResId);
  }*/
  /*if (nodeData[pc.currentFrame].nodes[n].hResId == hResId)
    debugPrintfEXT("res : %v2f, scale %v2i, lr %v2i",pc.resolution,hrLrScale, lrFragCoord);*/
  while( n != 0xffffffffu && count < MAX_FRAGMENTS && nodeData[pc.currentFrame].nodes[n].hResId == hResId) {
       frags[count] = nodeData[gl_ViewIndex*MAX_FRAMES_IN_FLIGHT+pc.currentFrame].nodes[n];
       n = frags[count].next;
       count++;
  }
  // Do the insertion sort
  for (uint i = 1; i < count; ++i)
  {
      NodeType insert = frags[i];
      uint j = i;
      while (j > 0 && insert.depth > frags[j - 1].depth)
      {
          frags[j] = frags[j-1];
          --j;
      }
      frags[j] = insert;
  }
  vec4 color = vec4(0, 0, 0, 0);
  for( int i = 0; i < count; i++)
  {
    /*color.rgb = frags[i].color.rgb * frags[i].color.a + color.rgb * (1 - frags[i].color.a);
    color.a = frags[i].color.a + color.a * (1 - frags[i].color.a);*/
    color = mix (color, frags[i].color, frags[i].color.a);
  }
  /*if (color.r != 0 || color.g != 0 || color.b != 0) 
    debugPrintfEXT("color : %v4f", color);*/
  fcolor = color;
}