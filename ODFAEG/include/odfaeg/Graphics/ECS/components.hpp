#ifndef ODFAEG_ECS_COMPONENTS_HPP
#define ODFAEG_ECS_COMPONENTS_HPP
#include "../../config.hpp"
#ifndef VULKAN
#include "../glCheck.h"
#include <GL/glew.h>
#else
#include <vulkan/vulkan.h>
#endif
#include "../../Math/vec4.h"
#include "../../Math/transformMatrix.h"
#include "../batcher.h"
#include "../renderWindow.h"
#include "../view.h"
#include "../rectangleShape.h"
#include "../../Core/ecs.hpp"

namespace odfaeg {
    namespace graphic {
        namespace ecs {
            enum WallType {
                    TOP_LEFT, TOP_RIGHT, BOTTOM_RIGHT, BOTTOM_LEFT, TOP_BOTTOM, RIGHT_LEFT, T_TOP, T_RIGHT, T_LEFT, T_BOTTOM, X, NB_WALL_TYPES
            };
            enum DrawMode {
                    NORMAL, INSTANCED, BASE_INSTANCED
            };
            struct ColliderComponent {
                physic::BoundingVolume* boundingVolume=nullptr;
                template <typename Archive>
                void serialize(Archive& ar) {
                    ar(boundingVolume);
                }
            };
            struct EntityInfoComponent {
                std::string groupName="";
                size_t animIndex=0;
                DrawMode drawMode = NORMAL;
                bool isSelected = false;
                template <typename Archive>
                void serialize(Archive& ar) {

                    ar(groupName);
                    //std::cout<<"groupName : "<<groupName<<std::endl;
                    ar(animIndex);
                    //std::cout<<"animIndex : "<<animIndex<<std::endl;
                    ar(drawMode);
                    //std::cout<<"drawMode : "<<animIndex<<std::endl;
                    ar(isSelected);
                    //std::cout<<"isSelected : "<<isSelected<<std::endl;

                }
            };
            struct WallTypeComponent {
                WallType wallType=WallType::TOP_LEFT;
                template <typename Archive>
                void serialize(Archive& ar) {
                    ar(wallType);
                    //std::cout<<"wallType : "<<wallType<<std::endl;

                }
            };
            struct TransformComponent {
                math::Vec3f position=math::Vec3f(0, 0, 0), size=math::Vec3f(0, 0, 0), center=math::Vec3f(0, 0, 0), scale=math::Vec3f(0, 0, 0), rotationAxis = math::Vec3f(0, 0, 1), origin=math::Vec3f(0, 0, 0);
                float rotation=0;
                TransformMatrix transformMatrix;
                physic::BoundingBox globalBounds, localBounds;
                template <typename Archive>
                void serialize(Archive& ar) {
                    ar(position);
                    //std::cout<<"position : "<<position<<std::endl;
                    ar(size);
                    //std::cout<<"size : "<<size<<std::endl;
                    ar(center);
                    //std::cout<<"center : "<<center<<std::endl;
                    ar(scale);
                    //std::cout<<"scale : "<<scale<<std::endl;
                    ar(rotationAxis);
                    //std::cout<<"rotation axis  : "<<rotationAxis<<std::endl;
                    ar(origin);
                    //std::cout<<"origin : "<<origin<<std::endl;
                    ar(rotation);
                    //std::cout<<"rotation : "<<rotation<<std::endl;
                    ar(transformMatrix);
                    //std::cout<<"transformMatrix : "<<std::endl;
                    ar(globalBounds);
                    //std::cout<<"globalBounds : "<<globalBounds.getPosition()<<globalBounds.getSize()<<std::endl;
                    ar(localBounds);
                    //std::cout<<"localBounds : "<<localBounds.getPosition()<<localBounds.getSize()<<std::endl;

                }
            };
            struct AnimationComponent {
                bool playing=false, loop=false;
                EntityId previousFrame=-1, currentFrame=-1, nextFrame=-1;
                std::size_t currentFrameIndex = 0;
                std::size_t interpLevels=1, interpPerc=0;
                EntityId interpolatedFrame=-1;
                float fr=0;
                sf::Clock clock;
                template <typename Archive>
                void serialize(Archive& ar) {
                    ar(playing);
                    //std::cout<<"playing : "<<playing<<std::endl;
                    ar(loop);
                    //std::cout<<"loop : "<<loop<<std::endl;
                    ar(previousFrame);
                    //std::cout<<"previous frame : "<<previousFrame<<std::endl;
                    ar(currentFrame);
                    //std::cout<<"current frame : "<<currentFrame<<std::endl;
                    ar(nextFrame);
                    //std::cout<<"next frame : "<<nextFrame<<std::endl;
                    ar(interpLevels);
                    //std::cout<<"interp level : "<<interpLevels<<std::endl;
                    ar(interpPerc);
                    //std::cout<<"interpPerc : "<<interpPerc<<std::endl;
                    ar(fr);
                    //std::cout<<"frame rate : "<<fr<<std::endl;

                }
            };
            struct SelectedAnimationComponent {
                size_t selectedAnimIndex=0;
                template <typename Archive>
                void serialize(Archive& ar) {
                    ar(selectedAnimIndex);
                }
            };
            struct MeshComponent {
                std::vector<Face> faces;
                template <typename Archive>
                void serialize(Archive& ar) {
                    ar(faces);
                    //std::cout<<"faces : "<<faces.size()<<std::endl;

                }
            };
            struct ShadowInfoComponent {
                math::Vec3f shadowCenter = math::Vec3f(0, 0, 0), shadowScale = math::Vec3f(1, 1, 1), shadowRotationAxis = math::Vec3f(0, 0, 1), shadowOrigin = math::Vec3f(0, 0, 0);
                float shadowRotationAngle = 0;
                template <typename Archive>
                void serialize(Archive& ar) {
                    ar(shadowCenter);
                    ar(shadowScale);
                    ar(shadowRotationAxis);
                    ar(shadowOrigin);
                    ar(shadowRotationAngle);
                }
            };
        }
    }
}
#endif
