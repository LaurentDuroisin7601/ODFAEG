#ifndef ODFAEG_ECS_COMPONENTS_HPP
#define ODFAEG_ECS_COMPOENNTS_HPP
namespace odfaeg {
    namespace graphic {
        struct ClonableComponent {
            EntityId tmpClonedRootId;
            EntityId tmpClonedParentId;
            bool isFirst;
        };
        struct SceneGridComponent {
            Grid grid;
            std::vector<std::vector<EntityId>> visibleEntities;
        };
        struct TransformComponent {
            math::Vec3f position;
            math::Vec3f scale;
            math::Vec3f rotationAxis;
            math::Vec3f size;
            float angle;
            math::TransformMatrix transform;
            physic::BoundingBox globalBounds, localBounds;
        };
        struct AnimationComponent {
            bool playing, loop;
            size_t previousFrame, currentFrame, nextFrame, nbFrames, boneIndex;
            size_t interpLevels, interpPerc;
            std::vector<Face> interpolatedFrameFaces;
            std::vector<std::vector<Face>> framesFaces;
        };
        struct BoneAnimationComponent {
            std::vector<size_t> selectedBonesIndexes;
            std::vector<AnimationComponent> boneAnimations;
        };
        struct PerPixelLinkedListBindlessTexSubRenderComponent  {
            size_t atomicBuffer, linkedListBuffer, clearBuff, headPtrTex, bindlessTexUBO, vboWorldMatrices;
            Vertexbuffer vb;
            std::array<VertexBuffer, Batcher::nbPrimitiveTypes> vbBindlessTex;
            std::vector<Instance> m_instances;
            std::vector<Instance> m_normals;
            Shader perPixelLinkedListNormal, perPixelLinkedListInstanced;
            std::vector<Face> facesToRender;
            std::vector<Face> instancesFacesToRender;
            RenderWindow& window;
        };
        struct PerPixelLinkedListRenderComponent {
            size_t linkedListBuffer, headPtrTex;
            Shader perPixelLinkedList;
            RenderWidnow& window;
        };
    }
}
#endif
