#ifndef ODFAEG_ECS_WORLD_HPP
#define ODFAEG_ECS_WORLD_HPP
#include "../../Core/ecs.hpp"
#include "systems.hpp"
#include "sceneManager.hpp"

#include "../../Core/entitySystem.h"
#include "../../Core/timer.h"
namespace odfaeg {
    namespace graphic {
        namespace ecs {
            class World {
                public :
                struct Cache {
                    std::vector<std::unique_ptr<core::EntitySystem>> eus; /**> holds every entity systems*/
                    std::vector<std::unique_ptr<core::Timer>> aus; /**> holds every timers.*/
                    std::vector<std::unique_ptr<SceneManager>> ems; /**> holds every entity managers*/
                };
                enum SystemsQueues {
                    MoveSystemQueueIndex, ResizeSystemQueueIndex, RotationSytemQueueIndex
                };
                World() {
                    /*nbEntities = 0;
                    nbEntitiesTypes = 0;*/
                    currentEntityManager = nullptr;
                }
                ComponentMapping& getComponentMapping() {
                    return componentMapping;
                }
                template <typename System>
                void addSystem ( System system, SystemsQueues queue, EntityFactory& factory) {
                    EntityId systemId = factory.createEntity();
                    if (queue >= systemQueueIds.size())
                        systemQueueIds.resize(queue+1);
                    systemQueueIds[queue].push_back(systemId);
                    addComponent(systemId, system, factory);
                }
                SceneManager* getCurrentSceneManager() {
                    return currentEntityManager;
                }
                std::vector<SceneManager*> getSceneManagers() {
                    std::vector<SceneManager*> ems;
                    for (unsigned int i = 0; i < cache.ems.size(); i++) {
                        ems.push_back(cache.ems[i].get());
                    }
                    return ems;
                }
                /**
                *    \fn std::vector<CellMap<E>*> getCasesMap()
                *    \brief get all the cells (containing the entities) of the entity manager.
                *    \return all the cells of the entity manager.
                */
                std::vector<Cell*> getCasesMap() {
                    if (currentEntityManager != nullptr)
                        return currentEntityManager->getCasesMap();
                    return std::vector<Cell*>();
                }
                /**
                *    \fn bool addEntity (E* entity)
                *    \brief add an entity into the engine.
                *    \param E* entity : tne entity to add.
                */
                /*void make_ids(Entity* entity) {
                    std::vector<Entity*> children = entity->getChildren();
                    for (unsigned int i = 0; i < children.size(); i++) {
                        make_ids(children[i]);
                    }
                    if (entity->getId() == -1) {
                        entity->setId(nbEntities);
                    }
                    nbEntities++;
                    int iType = getIntOfType(entity->getType());
                    if (iType == -1) {
                        types.insert(std::make_pair(nbEntitiesTypes, entity->getType()));
                        entity->setTypeInt(nbEntitiesTypes);
                        nbEntitiesTypes++;
                    } else {
                        entity->setTypeInt(iType);
                    }
                }*/
                bool addEntity (EntityId entity) {
                    if (currentEntityManager != nullptr) {
                        return currentEntityManager->addEntity(componentMapping, entity);
                    }
                    return false;
                }
                void addChild (EntityId parent, EntityId child) {
                    componentMapping.addChild(parent, child);
                }
                bool removeEntity(EntityId entity) {
                    if (currentEntityManager != nullptr)
                        return currentEntityManager->removeEntity(componentMapping, entity);
                    return false;
                }
                bool deleteEntity(EntityId entity) {
                    if (currentEntityManager != nullptr) {
                        componentMapping.remove(entity);
                        return currentEntityManager->deleteEntity(componentMapping, entity);
                    }
                    return false;
                }

                /** \fn getVisibleEntities (std::string expression)
                *   \brief get the visible entities of the given types.
                *   \param the types of the entities to get.
                *   \return the visible entities of the given types.
                */
                std::vector<graphic::ecs::EntityId> getVisibleEntities (std::string expression, EntityFactory& factory) {
                    if (currentEntityManager != nullptr)
                        return currentEntityManager->getVisibleEntities(expression, factory, componentMapping);
                    return std::vector<EntityId>();
                }
                /**
                *   \fn void moveEntity(E* entity, float x, float y, float z)
                *   \brief move an entity of the entity manager.
                *   \param E* entity : the entity to move.
                *   \param float x : the translation on x.
                *   \param float y : the translation on y.
                *   \param float z : the translation on z.
                */
                template <typename... Systems>
                void moveEntity(EntityId entity, float x, float y, float z){
                    if (currentEntityManager != nullptr) {
                       currentEntityManager->moveEntity(componentMapping, entity, x, y, z);
                       if (MoveSystemQueueIndex < systemQueueIds.size()) {
                           std::vector<EntityId> loadSystemId = systemQueueIds[MoveSystemQueueIndex];
                           std::vector<EntityId> entities;
                           entities.push_back(entity);
                           auto sTuple = toTuple(loadSystemId, std::make_index_sequence<sizeof... (Systems)>());
                           auto params = std::make_tuple(entities, componentMapping, sTuple, math::Vec3f(x, y, z));
                           MainSystem main;
                           componentMapping.template apply<TransformComponent>(main, loadSystemId, params);
                       }
                    }
                }
                template <size_t... I, typename... Systems>
                auto toTuple(std::vector<EntityId> entities, std::index_sequence<I...> seq) {
                    auto tuple = std::make_tuple(getComponent<std::tuple_element<I, std::tuple<Systems...>>>(entities[I])...);
                    return tuple;
                }
                /** \fn bool collide(E* entity)
                *   \brief check if the entity collides with another entity of the entity manager.
                *   \param E* entity : the entity to check with.
                */
                bool collide(EntityId entity) {
                    if (currentEntityManager != nullptr) {
                        return currentEntityManager->collide(componentMapping, entity);
                    }
                    return false;
                }
                bool collide(EntityId entity, math::Vec3f position) {
                    if (currentEntityManager != nullptr) {
                        return currentEntityManager->collide(componentMapping, entity, position);
                    }
                    return false;
                }
                bool collide(EntityId entity, math::Ray ray) {
                    if (currentEntityManager != nullptr) {
                        return currentEntityManager->collide(componentMapping, entity, ray);
                    }
                    return false;
                }
                /**\fn void generateMap(std::vector<E*> tGrounds, std::vector<E*> tWalls)
                *  \brief generate a random map with the given ground's tiles and wall's tiles. (used for the tests)
                *  the walls are set in the bounds of the map. (It doesn't generate a dunjon.)
                *  \param std::vector<E*> tGrounds : the tiles of the ground to generate.
                *  \param std::vector<E*> tWalls : the tiles of the wall to generate.
                *  \param BoundingBox zone : the zone of the map to generate.
                */
                void generate_map(std::vector<EntityId> tGround, std::vector<EntityId> tWall, math::Vec2f tileSize, physic::BoundingBox zone, EntityFactory& factory) {

                    if (currentEntityManager != nullptr) {
                        currentEntityManager->generate_map(componentMapping, tGround, tWall, tileSize, zone, factory);
                    }
                }
                /** \fn void drawOnComponents(std::string expression, int layer, sf::BlendMode mode = sf::BlendMode::BlendAlpha)
                *   \brief draw the entities on a render components.
                *   \param std::string expression : the types of the entities to draw.
                *   \param int layer : the layer of the component.
                *   \param sf::BlendMode mode : the blend mode.
                */
                void drawOnComponents(std::string expression, int layer, sf::BlendMode mode = sf::BlendAlpha) {
                    if (currentEntityManager != nullptr) {
                        currentEntityManager->drawOnComponents(expression, layer, mode);
                    }
                }
                 /** \fn void drawOnComponents(Drawable &drawable, int layer, sf::BlendMode mode = sf::BlendMode::BlendAlpha)
                *   \brief draw a drawable entity onto the component.
                *   \param Drawable& drawable : the drawable entity to draw.
                *   \param int layer : the layer of the component.
                *   \param RenderStates states : the render states.
                */
                void drawOnComponents(Drawable &drawable, int layer, RenderStates states = graphic::RenderStates::Default) {
                    if (currentEntityManager != nullptr) {
                        currentEntityManager->drawOnComponents(drawable, layer, states);
                    }
                }
                /** \fn std::vector<E*> getEntities(std::string expression)
                *   \brief get the entities types in the entity manager.
                *   \param the types of the entities to get.
                *   \return the entities to get.
                */
                std::vector<EntityId> getRootEntities(std::string expression) {
                    if (currentEntityManager != nullptr) {
                        return currentEntityManager->getRootEntities(componentMapping, expression);
                    }
                    return std::vector<EntityId>();
                }
                std::vector<EntityId> getEntities(std::string expression) {
                    if (currentEntityManager != nullptr) {
                        return currentEntityManager->getEntities(componentMapping, expression);
                    }
                    return std::vector<EntityId>();
                }
                std::vector<EntityId> getChildrenEntities(std::string expression) {
                    if (currentEntityManager != nullptr) {
                        return currentEntityManager->getChildrenEntities(componentMapping, expression);
                    }
                    return std::vector<EntityId>();
                }
                /** \fn std::vector<math::Vec3f> getPath(E* entity, math::Vec3f finalPos)
                *   \brief get the path between an entity and a position.
                *   \param E* entity : the Entity.
                *   \param math::Vec3f finalPos : the position.
                *   \return the path.
                */
                Cell* getGridCellAt(math::Vec3f pos) {
                    if (currentEntityManager != nullptr) {
                        return currentEntityManager->getGridCellAt(pos);
                    }
                    return nullptr;
                }
                std::vector<math::Vec2f> getPath(EntityId entity, math::Vec2f finalPos) {
                    std::vector<math::Vec2f> positions;
                    if (currentEntityManager != nullptr) {
                        if (getGridCellAt(finalPos) != nullptr) {

                            unsigned int cpt = 0;
                            math::Vec2f currentPos = getComponent<TransformComponent>(entity)->center;
                            positions.push_back(currentPos);
                            std::vector<math::Vec2f> visitedPositions;
                            std::vector<math::Vec2f> neightbourPositions;
                            while(collide(entity, finalPos) && cpt < 1000) {
                                for (int i = -50; i <= 50; i+= 50) {
                                    for (int j = -50; j <= 50; j+= 50) {
                                        if (i != 0 && j != 0) {
                                            math::Vec2f v(i, j);
                                            neightbourPositions.push_back(finalPos + v);
                                        }
                                    }
                                }
                                int distMin = neightbourPositions[0].computeDist(currentPos);
                                int indMin = 0;
                                for (unsigned int i = 1; i < neightbourPositions.size(); i++) {
                                    int dist = neightbourPositions[i].computeDist(currentPos);
                                    if (dist < distMin)  {
                                        distMin = dist;
                                        indMin = i;
                                    }
                                }
                                finalPos = neightbourPositions[indMin];
                                neightbourPositions.clear();
                                cpt++;

                            }
                            if (cpt == 1000) {
                                positions.clear();
                                return positions;
                            }
                            cpt = 0;
                            while(currentPos.computeDist(finalPos) > 50 && cpt < 1000) {
                                for (int i = -50; i <= 50; i+= 50) {
                                    for (int j = -50; j <= 50; j+= 50) {
                                        if (i != 0 && j != 0) {
                                            bool visited = false;
                                            math::Vec2f v(i, j);
                                            for (unsigned int n = 0; n < visitedPositions.size(); n++) {
                                                if (currentPos + v == visitedPositions[n])
                                                    visited = true;
                                            }
                                            if (!collide(entity, currentPos + v) && !visited) {
                                                neightbourPositions.push_back(currentPos + v);
                                            }
                                        }

                                    }
                                }
                                if (neightbourPositions.size() > 0) {
                                    int distMin = neightbourPositions[0].computeDist(finalPos);
                                    int indMin = 0;
                                    for (unsigned int i = 1; i < neightbourPositions.size(); i++) {
                                        int dist = neightbourPositions[i].computeDist(finalPos);
                                        if (dist < distMin)  {
                                            distMin = dist;
                                            indMin = i;
                                        }
                                    }
                                    currentPos = neightbourPositions[indMin];
                                    if (positions.size() > 1) {
                                        math::Vec2f v1 = positions[positions.size()-1] - positions[positions.size() - 2];
                                        math::Vec2f v2 = currentPos - positions[positions.size()-1];
                                        if (math::Math::abs(math::Math::toDegrees(v1.getAngleBetween(v2))) < 179)
                                            positions.push_back(currentPos);
                                    } else {
                                        positions.push_back(currentPos);
                                    }
                                    visitedPositions.push_back(currentPos);
                                    neightbourPositions.clear();
                                }
                                cpt++;
                            }
                            if (cpt == 1000) {
                                positions.clear();
                                return positions;
                            }
                            positions.push_back(finalPos);
                        }
                    }
                    return positions;
                }
                /** \fn CellMap<E>* getGridCellAt(math::Vec3f pos)
                *   \brief get the cell at the given position.
                *   \param math::Vec3f pos : the position.
                *   \return the cell.
                */

                void updateTimers() {
                    for (unsigned int i = 0; i < cache.aus.size(); i++) {
                        cache.aus[i]->update();
                    }
                }
                BaseChangementMatrix getBaseChangementMatrix() {
                    if (currentEntityManager != nullptr) {
                        return currentEntityManager->getBaseChangementMatrix();
                    }
                    return graphic::BaseChangementMatrix();
                }
                void addWorker(core::EntitySystem *eu) {
                    std::unique_ptr<core::EntitySystem> ptr;
                    ptr.reset(eu);
                    cache.eus.push_back(std::move(ptr));
                }
                void addTimer(core::Timer *au) {
                    std::unique_ptr<core::Timer> ptr;
                    ptr.reset(au);
                    cache.aus.push_back(std::move(ptr));
                }
                core::Timer* getTimer(std::string name) {
                    for (unsigned int i = 0; i < cache.aus.size(); i++) {
                        if (cache.aus[i]->getName() == name) {
                            return cache.aus[i].get();
                        }
                    }
                    return nullptr;
                }
                std::vector<core::Timer*> getTimers() {
                    std::vector<core::Timer*> timers;
                    for (unsigned int i = 0; i < cache.aus.size(); i++) {
                        timers.push_back(cache.aus[i].get());
                    }
                    return timers;
                }
                core::EntitySystem* getWorker(std::string name) {
                    for (unsigned int i = 0; i < cache.eus.size(); i++) {
                        if (cache.eus[i]->getName() == name) {
                            return cache.eus[i].get();
                        }
                    }
                    return nullptr;
                }
                std::vector<core::EntitySystem*> getWorkers() {
                    std::vector<core::EntitySystem*> workers;
                    for (unsigned int i = 0; i < cache.eus.size(); i++) {
                        workers.push_back(cache.eus[i].get());
                    }
                    return workers;
                }
                void update(std::string name="*") {
                    if (name == "*") {
                        for (unsigned int i = 0; i < cache.eus.size(); i++) {
                            cache.eus[i]->update();
                        }
                    } else {
                        for (unsigned int i = 0; i < cache.eus.size(); i++) {
                            if (cache.eus[i]->getName() == name) {
                                cache.eus[i]->update();
                            }
                        }
                    }
                }
                void addSceneManager(graphic::ecs::SceneManager* holder) {
                    std::unique_ptr<SceneManager> ptr;
                    ptr.reset(holder);
                    cache.ems.push_back(std::move(ptr));
                }
                void checkVisibleEntities(EntityFactory& factory) {
                    if (currentEntityManager != nullptr) {
                        currentEntityManager->checkVisibleEntities(factory, componentMapping);
                    }
                }
                std::vector<EntityId> getEntitiesInRect(physic::BoundingBox rect, std::string expression) {
                    if (currentEntityManager != nullptr) {
                        return currentEntityManager->getEntitiesInBox(componentMapping, rect, expression);
                    }
                    return std::vector<EntityId>();
                }
                void removeSceneManager (std::string emName) {
                    const auto itEmToRemove = std::find_if(cache.ems.begin(), cache.ems.end(), [&](auto& p) { return p.get()->getName() == emName; });
                    const bool found = (itEmToRemove != cache.ems.end());
                    if (found) {
                        cache.ems.erase(itEmToRemove);
                    }
                }
                void setCurrentSceneManager (std::string mapName) {

                    std::vector<SceneManager*> ems = getSceneManagers();
                    for (unsigned int i = 0; i < ems.size(); i++) {
                        std::string otherName = ems[i]->getName();
                        if (otherName == mapName) {
                           currentEntityManager = ems[i];
                        }
                    }
                }
                math::Vec3f getCoordinatesAt(math::Vec3f point) {
                    if (currentEntityManager != nullptr) {
                        return currentEntityManager->getCoordinatesAt(point);
                    }
                }
            private :
                Cache cache;
                graphic::ecs::SceneManager* currentEntityManager; /**> holds the current entity manager.*/
                ComponentMapping componentMapping;
                std::vector<std::vector<EntityId>> systemQueueIds;
            };
        }
    }
}
#endif
