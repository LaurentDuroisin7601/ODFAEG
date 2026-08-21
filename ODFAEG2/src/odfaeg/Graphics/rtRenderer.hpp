#ifndef ODFAEG_RT_RENDERER
#define ODFAEG_RT_RENDERER
namespace odfaeg {
    namespace graphic {
        class RTRenderer {
            public :
                struct GeometryOffset {
                    uint32_t vertexOffset;
                    uint32_t indexOffset;
                    uint32_t materialOffset;
                }
                struct MaterialData {
                    math::Vec2f uvScale;
                    math::Vec2f uvOffset;
                    unsigned int diffuseTextureIndex;
                    unsigned int specularTextureIndex;
                    unsigned int normalTextureIndex;
                    unsigned int metalnessTextureIndex;
                    unsigned int roughnessTextureIndex;
                    unsigned int aoTextureIndex;
                    unsigned int emissiveTextureIndex;
                    unsigned int materialType;
                    unsigned int materialSet;
                    unsigned int nbVertices;
                    unsigned int nbIndexes;
                    int instanceGroupId;
                    unsigned int vertsInstanceSet;
                    unsigned int materialId;
                    unsigned int nbBuffers;
                    unsigned int padding;
                };	
                RTRenderer(RenderTarget& parentRenderer, unsigned int layer, std::string typesToRenderExpression, int windowId=-1, bool usethread = false);
                void clear();
                void drawNextFrame();
                void draw();
            };
        };
    }
}
#endif