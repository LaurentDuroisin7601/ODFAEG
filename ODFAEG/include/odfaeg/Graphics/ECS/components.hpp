#ifndef ODFAEG_ECS_COMPONENTS_HPP
#define ODFAEG_ECS_COMPOENNTS_HPP
namespace odfaeg {
    namespace graphic {
        struct TransformComponent : IComponent {
            math::Vec3f position;
            math::Vec3f scale;
            math::Vec3f rotationAxis;
            float angle;
            math::TransformMatrix transform;
        };
        struct AnimationComponent  : IComponent{
            bool playing, loop;
            size_t previousFrame, currentFrame, nextFrame, nbFrames;
            size_t interpLevels, interpPerc;
            std::vector<Face> interpolatedFrameFaces;
            std::vector<std::vector<Face>> framesFaces;
        };
        struct PerPixelLinkedListRenderComponent  : IComponent {
            size_t atomicBuffer, linkedListBuffer, clearBuff, headPtrTex, bindlessTexUBO, vboWorldMatrices;
            Vertexbuffer vb;
            std::array<VertexBuffer, Batcher::nbPrimitiveTypes> vbBindlessTex;
            std::vector<Instance> m_instances;
            std::vector<Instance> m_normals;
            Shader perPixelLinkedList, perPixelLinkedListP2, perPixelLinkedList2;
            std::vector<Face> facesToRender;
            std::vector<Face> instancesFacesToRender;
            RenderWindow& window;
            bool subRender;
        };
    }
}
#endif
