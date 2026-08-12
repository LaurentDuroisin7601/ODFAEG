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
  uint count = 0;
  ivec2 hrLrScale = ivec2(pc.resolution.xy) / PPLL_RESOLUTION;
  ivec2 lrFragCoord = ivec2(gl_FragCoord.xy) / hrLrScale; 
  //debugPrintfEXT("current frame : %i", pc.currentFrame);
  uint n = imageLoad(headPointers[gl_ViewIndex*MAX_FRAMES_IN_FLIGHT+pc.currentFrame], lrFragCoord).r;  
  uint hResId = uint((int(gl_FragCoord.x) % hrLrScale.x) + (int(gl_FragCoord.y) % hrLrScale.y) * hrLrScale.x);
  //debugPrintfEXT("hrLres scale : %v2i, frag coords : %v2i, ids : %i,%i, global id : %i", hrLrScale, ivec2(gl_FragCoord.xy), int(gl_FragCoord.x) % hrLrScale.x, int(gl_FragCoord.y) % hrLrScale.y, hResId);
  /*if (nodeData[pc.currentFrame].nodes[n].hResId > 0 && nodeData[pc.currentFrame].nodes[n].hResId < 3) {
    debugPrintfEXT("h res id : %i", nodeData[pc.currentFrame].nodes[n].hResId);
  }*/
  //debugPrintfEXT("n : %i", n); 
  /*if (nodeData[pc.currentFrame].nodes[n].hResId == hResId)
    debugPrintfEXT("res : %v2f, scale %v2i, lr %v2i",pc.resolution,hrLrScale, lrFragCoord);*/
  while(n != 0xffffffffu && count < MAX_FRAGMENTS) { 
    
    NodeType frag = nodeData[gl_ViewIndex*MAX_FRAMES_IN_FLIGHT+pc.currentFrame].nodes[n]; 
    n = frag.next;
    //debugPrintfEXT("max nodes : %i, view index : %i, next : %i, hRes : %i", MAX_FRAGMENTS * pc.resolution.x * pc.resolution.y, gl_ViewIndex, n, hResId); 
    //if (hResId < 0 || hResId > 3)   
    //debugPrintfEXT("view index : %i, lrFragCoord : %v2i, n : %i, next : %i, hresIds, %i,%i", gl_ViewIndex, lrFragCoord, n, frag.next, frag.hResId, hResId);
    if (frag.hResId == hResId) {
       frags[count] = frag;       
       count++;   
    }  
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
  /*if (count == 0)
    debugPrintfEXT("count : %i", count);*/
  vec4 color = vec4(0, 0, 0, 1);
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