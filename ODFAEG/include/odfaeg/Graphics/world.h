#ifndef WORLD_H
#define WORLD_H

#include <SFML/Graphics.hpp>
#include "../Graphics/entityManager.h"

#include "../Core/entitySystem.h"
#include "../Core/timer.h"
#include "../Graphics/baseChangementMatrix.h"
#include "../Graphics/entityManager.h"
#include <cstdarg>
/**
 *\namespace odfaeg
 * the namespace of the Opensource Development Framework Adapted for Every Games.
 */
namespace odfaeg {
    namespace graphic {
        /**
        * \file world.h
        * \class World
        * \brief This class holds every entity managers and entity systems.
        * several entity managers can be hold, but only one can be selected.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        class  World {
            public :
                struct Cache {
                    std::vector<std::unique_ptr<core::EntitySystem>> eus; /**> holds every entity systems*/
                    std::vector<std::unique_ptr<core::Timer>> aus; /**> holds every timers.*/
                    std::vector<std::unique_ptr<SceneManager>> ems; /**> holds every entity managers*/
                };
                World() {
                    /*nbEntities = 0;
                    nbEntitiesTypes = 0;*/
                    currentEntityManager = nullptr;
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
                std::vector<graphic::CellMap*> getCasesMap() {
                    if (currentEntityManager != nullptr)
                        return currentEntityManager->getCasesMap();
                    return std::vector<graphic::CellMap*>();
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
                bool addEntity (Entity* entity) {
                    if (currentEntityManager != nullptr) {
                        //make_ids(entity);
                        return currentEntityManager->addEntity(entity);
                    }
                    return false;
                }
                bool removeEntity(Entity* entity) {
                    if (currentEntityManager != nullptr)
                        return currentEntityManager->removeEntity(entity);
                    return false;
                }
                bool deleteEntity(Entity* entity) {
                    if (currentEntityManager != nullptr) {
                        return currentEntityManager->deleteEntity(entity);
                    }
                    return false;
                }

                /** \fn getVisibleEntities (std::string expression)
                *   \brief get the visible entities of the given types.
                *   \param the types of the entities to get.
                *   \return the visible entities of the given types.
                */
                std::vector<graphic::Entity*> getVisibleEntities (std::string expression, EntityFactory& factory) {
                    if (currentEntityManager != nullptr)
                        return currentEntityManager->getVisibleEntities(expression, factory);
                    return std::vector<Entity*>();
                }
                /**
                *   \fn void moveEntity(E* entity, float x, float y, float z)
                *   \brief move an entity of the entity manager.
                *   \param E* entity : the entity to move.
                *   \param float x : the translation on x.
                *   \param float y : the translation on y.
                *   \param float z : the translation on z.
                */
                void moveEntity(Entity* entity, float x, float y, float z){
                    if (currentEntityManager != nullptr) {
                       currentEntityManager->moveEntity(entity, x, y, z);
                    }
                }
                /** \fn bool collide(E* entity)
                *   \brief check if the entity collides with another entity of the entity manager.
                *   \param E* entity : the entity to check with.
                */
                bool collide(Entity* entity) {
                    if (currentEntityManager != nullptr) {
                        return currentEntityManager->collide(entity);
                    }
                    return false;
                }
                bool collide(Entity* entity, math::Vec3f position) {
                    if (currentEntityManager != nullptr) {
                        return currentEntityManager->collide(entity, position);
                    }
                    return false;
                }
                bool collide(Entity* entity, math::Ray ray) {
                    if (currentEntityManager != nullptr) {
                        return currentEntityManager->collide(entity, ray);
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
                void generate_map(std::vector<Tile*> tGround, std::vector<g2d::Wall*> tWall, math::Vec2f tileSize, physic::BoundingBox zone, bool terrain3D, EntityFactory& factory) {

                    if (currentEntityManager != nullptr) {
                        currentEntityManager->generate_map(tGround, tWall, tileSize, zone, terrain3D, factory);
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
                std::vector<Entity*> getRootEntities(std::string expression) {
                    if (currentEntityManager != nullptr) {
                        return currentEntityManager->getRootEntities(expression);
                    }
                    return std::vector<Entity*>();
                }
                std::vector<Entity*> getEntities(std::string expression) {
                    if (currentEntityManager != nullptr) {
                        return currentEntityManager->getEntities(expression);
                    }
                    return std::vector<Entity*>();
                }
                std::vector<Entity*> getChildrenEntities(std::string expression) {
                    if (currentEntityManager != nullptr) {
                        return currentEntityManager->getChildrenEntities(expression);
                    }
                    return std::vector<Entity*>();
                }
                /** \fn std::vector<math::Vec3f> getPath(E* entity, math::Vec3f finalPos)
                *   \brief get the path between an entity and a position.
                *   \param E* entity : the Entity.
                *   \param math::Vec3f finalPos : the position.
                *   \return the path.
                */
                std::vector<math::Vec2f> getPath(Entity* entity, math::Vec2f finalPos) {
                    std::vector<math::Vec2f> positions;
                    if (currentEntityManager != nullptr) {
                        if (getGridCellAt(finalPos) != nullptr) {

                            unsigned int cpt = 0;
                            math::Vec2f currentPos = entity->getCenter();
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
                graphic::CellMap* getGridCellAt(math::Vec3f pos) {
                    if (currentEntityManager != nullptr) {
                        return currentEntityManager->getGridCellAt(pos);
                    }
                    return nullptr;
                }
                void updateTimers() {
                    for (unsigned int i = 0; i < cache.aus.size(); i++) {
                        cache.aus[i]->update();
                    }
                }
                graphic::BaseChangementMatrix getBaseChangementMatrix() {
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
                void addSceneManager(graphic::SceneManager* holder) {
                    std::unique_ptr<SceneManager> ptr;
                    ptr.reset(holder);
                    cache.ems.push_back(std::move(ptr));
                }
                void checkVisibleEntities(EntityFactory& factory) {
                    if (currentEntityManager != nullptr) {
                        currentEntityManager->checkVisibleEntities(factory);
                    }
                }
                std::vector<Entity*> getEntitiesInRect(physic::BoundingBox rect, std::string expression) {
                    if (currentEntityManager != nullptr) {
                        return currentEntityManager->getEntitiesInBox(rect, expression);
                    }
                    return std::vector<Entity*>();
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
                Entity* getEntity(int id) {
                    if (currentEntityManager != nullptr) {
                        return currentEntityManager->getEntity(id);
                    }
                    return nullptr;
                }
                Entity* getEntity(std::string name) {
                    if (currentEntityManager != nullptr) {
                        return currentEntityManager->getEntity(name);
                    }
                    return nullptr;
                }
                math::Vec3f getCoordinatesAt(math::Vec3f point) {
                    if (currentEntityManager != nullptr) {
                        return currentEntityManager->getCoordinatesAt(point);
                    }
                }
            private :
                Cache cache;
                graphic::SceneManager* currentEntityManager; /**> holds the current entity manager.*/
        };
    }
}
#endif
