#ifndef MODEL_FACTORY_HPP
#define MODEL_FACTORY_HPP
#include "../../Core/ecs.hpp"
#include "../rectangleShape.h"
#include "../2D/ambientLight.h"
#include <GLFW/glfw3.h>
#include <set>
#include "world.hpp"
namespace odfaeg {
    namespace graphic {
        namespace ecs {
        class ODFAEG_GRAPHICS_API ModelFactory {
                public :
                static EntityId createTileModel(ComponentMapping& componentMapping, const Texture *image, math::Vec3f position, math::Vec3f size, sf::IntRect subRect,  sf::Color color = sf::Color::White) {
                    EntityId tile = componentMapping.getEntityFactory().createEntity("E_TILE");
                    EntityInfoComponent eic;
                    eic.id = componentMapping.getEntityFactory().getNbEntities() - 1;
                    eic.groupName = "E_TILE";
                    TransformComponent tc;
                    TransformMatrix tm;
                    tc.localBounds = physic::BoundingBox(0, 0, 0, size.x, size.y, size.z);
                    tc.position = position;
                    tc.origin = size * 0.5f;
                    tc.center = position + tc.origin;
                    tm.setOrigin(tc.origin);
                    tm.setRotation(math::Vec3f::zAxis, 0);
                    tm.setTranslation(tc.center);
                    math::Vec3f scale(1.f, 1.f, 1.f);
                    tm.setScale(scale);
                    tc.globalBounds = tc.localBounds.transform(tm);
                    tc.transformMatrix = tm;
                    tc.size = size;
                    MeshComponent mesh;
                    VertexArray va(sf::Quads, 4, nullptr, tile);
                    Vertex v1(sf::Vector3f(0, 0, 0), color);
                    Vertex v2(sf::Vector3f(size.x, 0, 0), color);
                    Vertex v3(sf::Vector3f(size.x, size.y, 0), color);
                    Vertex v4(sf::Vector3f(0, size.y, 0), color);
                    v1.texCoords = sf::Vector2f(subRect.left, subRect.top);
                    v2.texCoords = sf::Vector2f(subRect.left + subRect.width, subRect.top);
                    v3.texCoords = sf::Vector2f(subRect.left + subRect.width, subRect.top + subRect.height);
                    v4.texCoords = sf::Vector2f(subRect.left, subRect.top + subRect.height);
                    //std::cout<<"tex coords : "<<v2.texCoords.x<<" "<<v2.texCoords.y<<std::endl;
                    //v1.color = v2.color = v3.color = v4.color = color;


                    va[0] = v1;
                    va[1] = v2;
                    va[2] = v3;
                    va[3] = v4;
                    Material material;
                    material.addTexture(image, sf::IntRect(0, 0, 0, 0));

                    Face face(va, material, tm);
                    mesh.faces.push_back(face);
                    componentMapping.addComponent(tile, tc);
                    componentMapping.addComponent(tile, eic);
                    componentMapping.addComponent(tile, mesh);
                    return tile;
                }
                static EntityId createBigTileModel(ComponentMapping& componentMapping, math::Vec3f position, math::Vec3f size) {
                    EntityId bigTile = componentMapping.getEntityFactory().createEntity("E_BIGTILE");
                    EntityInfoComponent eic;
                    eic.id = componentMapping.getEntityFactory().getNbEntities() - 1;
                    eic.groupName = "E_BIGTILE";
                    TransformMatrix tm;
                    TransformComponent tc;
                    tc.localBounds = physic::BoundingBox(0, 0, 0, size.x, size.y, size.z);
                    tc.position = position;
                    tc.size = size;
                    tc.origin = tc.size * 0.5f;
                    tc.center = position + tc.origin;
                    tm.setOrigin(tc.origin);
                    tm.setRotation(math::Vec3f::zAxis, 0);
                    tm.setTranslation(tc.center);
                    math::Vec3f scale(1.f, 1.f, 1.f);
                    tm.setScale(scale);
                    tc.globalBounds = tc.localBounds.transform(tm);
                    tc.transformMatrix = tm;
                    componentMapping.addComponent(bigTile, eic);
                    componentMapping.addComponent(bigTile, tc);
                    return bigTile;
                }
                static EntityId createWallModel(ComponentMapping& componentMapping, WallType type, EntityId tile, World& world) {
                    EntityId wall = componentMapping.getEntityFactory().createEntity("E_WALL");
                    EntityInfoComponent eic;
                    eic.id = componentMapping.getEntityFactory().getNbEntities() - 1;
                    eic.groupName = "E_WALL";
                    TransformComponent tc = *componentMapping.getComponent<TransformComponent>(tile);
                    WallTypeComponent wtc;
                    wtc.wallType = type;
                    ShadowInfoComponent sic;
                    Light* light = &g2d::AmbientLight::getAmbientLight();
                    float sy = light->getHeight() / (light->getHeight() * 0.75f);
                    sic.shadowScale = math::Vec3f(1, sy, 1);
                    int c = tc.size.y * sy;
                    sic.shadowCenter = math::Vec3f(0, 0, -c);
                    componentMapping.addComponent(wall, eic);
                    componentMapping.addComponent(wall, tc);
                    componentMapping.addComponent(wall, wtc);
                    componentMapping.addComponent(wall, sic);
                    world.addChild(wall, tile);
                    return wall;
                }
                static EntityId createDecorModel(ComponentMapping& componentMapping, EntityId tile, World& world) {
                    EntityId decor = componentMapping.getEntityFactory().createEntity("E_DECOR");
                    EntityInfoComponent eic;
                    eic.id = componentMapping.getEntityFactory().getNbEntities() - 1;
                    eic.groupName = "E_DECOR";
                    TransformComponent tc = *componentMapping.getComponent<TransformComponent>(tile);
                    ShadowInfoComponent sic;
                    Light* light = &g2d::AmbientLight::getAmbientLight();
                    float sy = light->getHeight() / (light->getHeight() * 0.75f);
                    sic.shadowScale = math::Vec3f(1, sy, 1);
                    int c = tc.size.y * sy;
                    sic.shadowCenter = math::Vec3f(0, 0, -c);
                    componentMapping.addComponent(decor, eic);
                    componentMapping.addComponent(decor, tc);
                    componentMapping.addComponent(decor, sic);
                    world.addChild(decor, tile);
                    return decor;
                }
                static EntityId createAnimationModel(ComponentMapping& componentMapping, float fr, math::Vec3f position, math::Vec3f size, bool loop,std::vector<EntityId> frames, World& world) {
                    EntityId animation = componentMapping.getEntityFactory().createEntity("E_ANIMATION");
                    EntityInfoComponent eic;
                    eic.id = componentMapping.getEntityFactory().getNbEntities() - 1;
                    eic.groupName = "E_ANIMATION";
                    TransformMatrix tm;
                    TransformComponent tc;
                    tc.localBounds = physic::BoundingBox(0, 0, 0, size.x, size.y, size.z);
                    tc.position = position;
                    tc.size = size;
                    tc.origin = tc.size * 0.5f;
                    tc.center = tc.position + tc.origin;
                    tm.setOrigin(tc.origin);
                    tm.setRotation(math::Vec3f::zAxis, 0);
                    tm.setTranslation(tc.center);
                    math::Vec3f scale(1.f, 1.f, 1.f);
                    tm.setScale(scale);
                    tc.globalBounds = tc.localBounds.transform(tm);
                    tc.transformMatrix = tm;
                    AnimationComponent ac;
                    ac.fr = fr;
                    ac.playing = true;
                    ac.loop = loop;
                    if (frames.size() > 0) {
                        ac.currentFrame = frames[ac.currentFrameIndex];
                        ac.previousFrame = (ac.currentFrameIndex - 1 < 0) ? frames[frames.size()-1] : frames[ac.currentFrameIndex-1];
                        ac.nextFrame = (ac.currentFrameIndex >= frames.size()) ? frames[0] : frames[ac.currentFrameIndex+1];
                        for (unsigned int i = 0; i < frames.size(); i++) {
                            world.addChild(animation, frames[i]);
                        }
                    }
                    ShadowInfoComponent sic;
                    Light* light = &g2d::AmbientLight::getAmbientLight();
                    float sy = light->getHeight() / (light->getHeight() * 0.75f);
                    sic.shadowScale = math::Vec3f(1, sy, 1);
                    int c = tc.size.y * sy;
                    sic.shadowCenter = math::Vec3f(0, 0, -c);
                    componentMapping.addComponent(animation, eic);
                    componentMapping.addComponent(animation, tc);
                    componentMapping.addComponent(animation, ac);
                    componentMapping.addComponent(animation, sic);
                    return animation;
                }
            };
        }
    }
}
#endif
