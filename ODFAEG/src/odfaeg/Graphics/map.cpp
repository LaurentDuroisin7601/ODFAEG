#include "../../../include/odfaeg/Graphics/map.h"
#include "../../../include/odfaeg/Graphics/rectangleShape.h"
#include "../../../include/odfaeg/Physics/boundingEllipsoid.h"
#include <iostream>
#include <climits>
#include "../../../include/odfaeg/Core/singleton.h"
#include "../../../include/odfaeg/Graphics/tGround.h"
#include "../../../include/odfaeg/Graphics/boneAnimation.hpp"
#include "../../../include/odfaeg/Graphics/application.h"
namespace odfaeg {
    namespace graphic {
    using namespace std;
    using namespace g2d;
        Scene::Scene() : SceneManager () {
            frcm = nullptr;
            cellWidth = cellHeight = cellDepth = 0;
            gridMap = nullptr;
            name = "";
        }
        Scene::Scene (RenderComponentManager* frcm, std::string name, int cellWidth, int cellHeight, int cellDepth) : SceneManager(), frcm(frcm), cellWidth(cellWidth), cellHeight(cellHeight), cellDepth(cellDepth) {
            gridMap = new GridMap(cellWidth, cellHeight, cellDepth);
            id = 0;
            version = 1;
            this->name = name;
            diagSize = math::Math::sqrt(math::Math::power(cellWidth, 2) + math::Math::power(cellHeight, 2));
        }
        void Scene::setRenderComponentManager(RenderComponentManager* frcm) {
            this->frcm = frcm;
        }
        int Scene::getCellWidth() {
            return cellWidth;
        }
        int Scene::getCellHeight() {
            return cellHeight;
        }
         void Scene::generate_labyrinthe (std::vector<Tile*> tGround, std::vector<Wall*> walls, math::Vec2f tileSize, physic::BoundingBox &rect, bool laby3D, EntityFactory& factory) {
            int startX = rect.getPosition().x / tileSize.x * tileSize.x;
            int startY = rect.getPosition().y / tileSize.y * tileSize.y;
            int endX = (rect.getPosition().x + rect.getWidth()) / tileSize.x * tileSize.x;
            int endY = (rect.getPosition().y + rect.getHeight()) / tileSize.y * tileSize.y;
            BigTile *bt;
            bt = factory.make_entity<BigTile>(math::Vec3f(startX, startY, startY + endY * 0.5f), factory);
            addEntity(bt);
            unsigned int i, j;
            for (int y = startY, j = 0; y < endY; y+= tileSize.y, j++) {
                for (int x = startX, i = 0; x < endX; x+= tileSize.x, i++) {
                    math::Vec3f projPos = gridMap->getBaseChangementMatrix().changeOfBase(math::Vec3f (x - startX, y - startY, 0));
                    math::Vec2f pos (projPos.x + startX, projPos.y + startY);
                    if (x == startX && y == startY) {
                        Entity *w = walls[Wall::TOP_LEFT]->clone();
                        w->setPosition(math::Vec3f(pos.x, pos.y, pos.y + walls[Wall::TOP_LEFT]->getSize().y * 0.5f));
                        addEntity(w);
                        gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, 0))->setPassable(false);
                    } else if (x == endX && y == startY) {
                        Entity *w = walls[Wall::TOP_RIGHT]->clone();
                        w->setPosition(math::Vec3f(pos.x, pos.y, pos.y + walls[Wall::TOP_RIGHT]->getSize().y * 0.5f));
                        addEntity(w);
                        gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, 0))->setPassable(false);
                    } else if (y == endY && x == endX) {
                        Entity *w = walls[Wall::BOTTOM_RIGHT]->clone();
                        w->setPosition(math::Vec3f(pos.x, pos.y, pos.y + walls[Wall::BOTTOM_RIGHT]->getSize().y * 0.5f));
                        addEntity(w);
                        gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, 0))->setPassable(false);
                    } else if (x == startX && y == endY) {
                        Entity *w = walls[Wall::BOTTOM_LEFT]->clone();
                        w->setPosition(math::Vec3f(pos.x, pos.y, pos.y + walls[Wall::BOTTOM_LEFT]->getSize().y * 0.5f));
                        addEntity(w);
                        gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, 0))->setPassable(false);
                    } else if (y == startY && j % 2 != 0) {
                        Entity *w = walls[Wall::TOP_BOTTOM]->clone();
                        w->setPosition(math::Vec3f(pos.x, pos.y, pos.y + walls[Wall::TOP_BOTTOM]->getSize().y * 0.5f));
                        addEntity(w);
                        gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, 0))->setPassable(false);
                    } else if (y == startY && j % 2 == 0) {
                        Entity *w = walls[Wall::T_TOP]->clone();
                        w->setPosition(math::Vec3f(pos.x, pos.y, pos.y + walls[Wall::T_TOP]->getSize().y * 0.5f));
                        addEntity(w);
                        gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, 0))->setPassable(false);
                    } else if (x == endX && j % 2 != 0) {
                        Entity *w = walls[Wall::RIGHT_LEFT]->clone();
                        w->setPosition(math::Vec3f(pos.x, pos.y, pos.y + walls[Wall::RIGHT_LEFT]->getSize().y * 0.5f));
                        addEntity(w);
                        gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, 0))->setPassable(false);
                    } else if (x == endX && j % 2 == 0) {
                        Entity *w = walls[Wall::T_RIGHT]->clone();
                        w->setPosition(math::Vec3f(pos.x, pos.y, pos.y + walls[Wall::T_RIGHT]->getSize().y * 0.5f));
                        addEntity(w);
                        gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, 0))->setPassable(false);
                    } else if(y == endY && i % 2 != 0) {
                        Entity *w = walls[Wall::TOP_BOTTOM]->clone();
                        w->setPosition(math::Vec3f(pos.x, pos.y, pos.y + walls[Wall::TOP_BOTTOM]->getSize().y * 0.5f));
                        addEntity(w);
                        gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, 0))->setPassable(false);
                    } else if (y == endY && i % 2 == 0) {
                        Entity *w = walls[Wall::T_BOTTOM]->clone();
                        w->setPosition(math::Vec3f(pos.x, pos.y, pos.y + walls[Wall::T_BOTTOM]->getSize().y * 0.5f));
                        addEntity(w);
                        gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, 0))->setPassable(false);
                    } else if (x == startX && j % 2 != 0) {
                        Entity *w = walls[Wall::RIGHT_LEFT]->clone();
                        w->setPosition(math::Vec3f(pos.x, pos.y, pos.y + walls[Wall::RIGHT_LEFT]->getSize().y * 0.5f));
                        addEntity(w);
                        gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, 0))->setPassable(false);
                    } else if (x == startX && j % 2 == 0) {
                        Entity *w = walls[Wall::T_LEFT]->clone();
                        w->setPosition(math::Vec3f(pos.x, pos.y, pos.y + walls[Wall::T_LEFT]->getSize().y * 0.5f));
                        addEntity(w);
                        gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, 0))->setPassable(false);
                    } else if (j % 2 != 0 && i % 2 == 0) {
                        Entity *w = walls[Wall::RIGHT_LEFT]->clone();
                        w->setPosition(math::Vec3f(pos.x, pos.y, pos.y + walls[Wall::RIGHT_LEFT]->getSize().y * 0.5f));
                        addEntity(w);
                        gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, 0))->setPassable(false);
                    } else if (j % 2 == 0 && i % 2 != 0) {
                        Entity *w = walls[Wall::TOP_BOTTOM]->clone();
                        w->setPosition(math::Vec3f(pos.x, pos.y, pos.y + walls[Wall::TOP_BOTTOM]->getSize().y * 0.5f));
                        addEntity(w);
                        gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, 0))->setPassable(false);
                    } else if (j % 2 == 0 && i % 2 == 0) {
                        Entity *w = walls[Wall::X]->clone();
                        w->setPosition(math::Vec3f(pos.x, pos.y, pos.y + walls[Wall::X]->getSize().y * 0.5f));
                        addEntity(w);
                        gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, 0))->setPassable(false);
                    }
                }
            }
            std::vector<CellMap*> visited;
            std::vector<math::Vec2f> dirs;
            int n = 0, cx = startX + tileSize.x, cy = startY + tileSize.y;
            while (n < i * j) {
                for (unsigned int i = 0; i < 4; i++) {
                    int x = cx, y = cy;
                    math::Vec2f dir;
                    if (i == 0) {
                        dir = math::Vec2f (1, 0);
                    } else if (i == 1) {
                        dir = math::Vec2f (0, 1);
                    } else if (i == 2) {
                        dir = math::Vec2f (-1, 0);
                    } else {
                        dir = math::Vec2f (0, -1);
                    }
                    x += dir.x * tileSize.x * 2;
                    y += dir.y * tileSize.y * 2;
                    math::Vec3f projPos = gridMap->getBaseChangementMatrix().changeOfBase(math::Vec3f (x - startX, y - startY, 0));
                    math::Vec2f pos (projPos.x + startX, projPos.y + startY);
                    CellMap* cell = gridMap->getGridCellAt(math::Vec2f(pos.x, pos.y));
                    if (cell != nullptr) {
                        dirs.push_back(dir);
                    }
                }
                math::Vec2f dir = dirs[math::Math::random(0, dirs.size())];
                int x = cx + dir.x * tileSize.x;
                int y = cy + dir.y * tileSize.y;
                math::Vec3f projPos = gridMap->getBaseChangementMatrix().changeOfBase(math::Vec3f (x - startX, y - startY, 0));
                math::Vec2f pos (projPos.x + startX, projPos.y + startY);
                CellMap* cell = gridMap->getGridCellAt(math::Vec2f(pos.x, pos.y));
                cell->removeEntity("E_WALL");
                cell->setPassable(true);
                cx += dir.x * tileSize.x * 2;
                cy += dir.y * tileSize.y * 2;
                x = cx, y = cy;
                projPos = gridMap->getBaseChangementMatrix().changeOfBase(math::Vec3f (x - startX, y - startY, 0));
                pos = math::Vec2f (projPos.x + startX, projPos.y + startY);
                cell = gridMap->getGridCellAt(math::Vec2f(pos.x, pos.y));
                bool contains = false;
                for (unsigned int j = 0; j < visited.size() && !contains; j++) {
                    if (visited[j] == cell)
                        contains = true;
                }
                if (!contains) {
                    n++;
                    visited.push_back(cell);
                }
            }
         }
        void Scene::generate_map(std::vector<Tile*> tGround, std::vector<Wall*> walls, math::Vec2f tileSize, physic::BoundingBox &rect, bool terrain3D, EntityFactory& factory) {
            int startX = rect.getPosition().x / tileSize.x * tileSize.x;
            int startY = rect.getPosition().y / tileSize.y * tileSize.y;
            int endX = (rect.getPosition().x + rect.getWidth()) / tileSize.x * tileSize.x;
            int endY = (rect.getPosition().y + rect.getHeight()) / tileSize.y * tileSize.y;
            BigTile *bt;
            if (!terrain3D)
                bt = factory.make_entity<BigTile>(math::Vec3f(startX, startY, startY + (endY - startY) * 0.5f), factory);
            else
                bt = factory.make_entity<BigTile>(math::Vec3f(startX, startY, rect.getPosition().z),factory, tileSize,rect.getWidth() / tileSize.x);
            bt->setSize(rect.getSize());
            //bt->setCenter(math::Vec3f(rect.getCenter().x, rect.getCenter().y, rect.getPosition().z));
            //Positions de d\E9part et d'arriv\E9es en fonction de la taille, de la position et de la taille des cellules de la map.
            for (int y = startY; y < endY;  y+=tileSize.y) {
                for (int x = startX; x < endX; x+=tileSize.x) {

                    //On projete les positions en fonction de la projection du jeux.
                    math::Vec3f projPos = gridMap->getBaseChangementMatrix().changeOfBase(math::Vec3f (x - startX, y - startY, 0));
                    math::Vec2f pos (projPos.x + startX, projPos.y + startY);
                    //std::cout<<"pos : "<<pos<<std::endl;
                    //Mur du coin en haut \E0 gauche.
                    if (x == startX && y == startY && walls.size() >= 11) {
                        if (walls[Wall::TOP_LEFT] != nullptr) {
                            Entity *w = walls[Wall::TOP_LEFT]->clone();
                            w->setPosition(math::Vec3f(pos.x, pos.y, pos.y + walls[Wall::TOP_LEFT]->getSize().y * 0.5f));
                            //std::cout<<"position top right : "<<w->getPosition()<<std::endl;
                            addEntity(w);
                            gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, w->getPosition().z))->setPassable(false);
                        }

                        //Mur du coin en haut \E0 droite.
                    } else if (x == endX - tileSize.x && y == startY && walls.size() >= 11) {
                        if (walls[Wall::TOP_RIGHT] != nullptr) {
                            Entity *w = walls[Wall::TOP_RIGHT]->clone();
                            w->setPosition(math::Vec3f(pos.x, pos.y, pos.y + walls[Wall::TOP_RIGHT]->getSize().y * 0.5f));
                            //std::cout<<"position top right : "<<w->getPosition()<<std::endl;
                            addEntity(w);
                            gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, w->getPosition().z))->setPassable(false);
                        }
                        //Mur du coin en bas \E0 droite.
                    } else if (x == endX - tileSize.x && y == endY - tileSize.y && walls.size() >= 11) {
                        if (walls[Wall::BOTTOM_RIGHT] != nullptr) {
                            Entity *w = walls[Wall::BOTTOM_RIGHT]->clone();
                            w->setPosition(math::Vec3f(pos.x, pos.y, pos.y + walls[Wall::BOTTOM_RIGHT]->getSize().y * 0.5f));
                            addEntity(w);
                            gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, w->getPosition().z))->setPassable(false);
                        }
                    } else if (x == startX && y == endY - tileSize.y && walls.size() >= 11) {
                        if (walls[Wall::BOTTOM_LEFT] != nullptr) {
                            Entity *w = walls[Wall::BOTTOM_LEFT]->clone();
                            w->setPosition(math::Vec3f(pos.x, pos.y, pos.y + walls[Wall::BOTTOM_LEFT]->getSize().y * 0.5f));
                            //std::cout<<"position bottom left : "<<w->getPosition()<<std::endl;
                            addEntity(w);
                            gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, w->getPosition().z))->setPassable(false);
                        }
                    } else if ((y == startY || y == endY - tileSize.y) && walls.size() >= 11) {
                        if (walls[Wall::TOP_BOTTOM] != nullptr) {
                            Entity *w = walls[Wall::TOP_BOTTOM]->clone();
                            w->setPosition(math::Vec3f(pos.x, pos.y, pos.y + walls[Wall::TOP_BOTTOM]->getSize().y * 0.5f));
                            addEntity(w);
                            if (y == endY - tileSize.y) {
                                int i = math::Math::random(tGround.size());
                                Entity *tile = tGround[i]->clone();
                                tile->setPosition(math::Vec3f(pos.x, pos.y, pos.y + tile->getSize().y * 0.5f));
                                bt->addTile(tile);
                            }
                            gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, w->getPosition().z))->setPassable(false);
                        }
                    } else if ((x == startX || x == endX - tileSize.x) && walls.size() >= 11) {
                        if (walls[Wall::RIGHT_LEFT] != nullptr) {
                            Entity *w = walls[Wall::RIGHT_LEFT]->clone();

                            w->setPosition(math::Vec3f(pos.x, pos.y, pos.y + walls[Wall::RIGHT_LEFT]->getSize().y * 0.5f));
                            addEntity(w);
                            if (x == endX - tileSize.x) {
                                int i = math::Math::random(tGround.size());
                                Entity *tile = tGround[i]->clone();
                                tile->setPosition(math::Vec3f(pos.x, pos.y, pos.y + tile->getSize().y * 0.5f));
                                bt->addTile(tile);
                            }
                            gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, w->getPosition().z))->setPassable(false);
                        }
                    } else {
                        Entity* tile;
                        if (tGround.size() > 0)  {
                            int i = math::Math::random(tGround.size());
                            tile = tGround[i]->clone();
                            tile->setPosition(math::Vec3f(pos.x, pos.y, pos.y + tile->getSize().y * 0.5f));
                            //std::cout<<"add tile : "<<tile->getPosition()<<std::endl;
                        } else {
                            tile = factory.make_entity<Tile>(nullptr, math::Vec3f(pos.x, pos.y, pos.y + tileSize.y * 0.5f), math::Vec3f(tileSize.x, tileSize.y, 0), sf::IntRect(0, 0, tileSize.x, tileSize.y), factory);
                        }
                        if (terrain3D) {
                            float heights[4];
                            for (unsigned int j = 0; j < sizeof(heights) / sizeof(float); j++) {
                                heights[j] = math::Math::random(rect.getPosition().z, rect.getPosition().z + rect.getDepth());
                            }
                            bt->addTile(tile, math::Vec2f(pos.x, pos.y), heights);
                        } else {
                            bt->addTile(tile);
                        }
                    }
                }
            }
            addEntity(bt);
        }
        vector<math::Vec3f> Scene::getPath(Entity* entity, math::Vec3f finalPos) {
            return gridMap->getPath(entity, finalPos);
        }
        void Scene::setName (string name) {
            this->name = name;
        }
        string Scene::getName() {
            return name;
        }
        void Scene::setId (int id) {
            this->id = id;
        }
        int Scene::getId () {
            return id;
        }
        void Scene::setVersion (int version) {
            this->version = version;
        }
        int Scene::getVersion () {
            return version;
        }
        void Scene::removeComptImg (const void* resource) {
            map<const void*, int>::iterator it;
            it = compImages.find(resource);
            if (it != compImages.end()) {
                compImages.erase(it);
            }
        }
        void Scene::increaseComptImg(const void* resource) {
            map<const void*, int>::iterator it;
            it = compImages.find(resource);
            if (it != compImages.end()) {
                it->second = it->second + 1;
            } else {
                compImages.insert(pair<const void*, int> (resource, 1));
            }
        }
        void Scene::decreaseComptImg (const void* resource) {
            map<const void*, int>::iterator it;
            it = compImages.find(resource);
            if (it != compImages.end() && it->second != 0) {
                it->second = it->second - 1;
            }
        }
        int Scene::getCompImage(const void* resource) {
            map<const void*, int>::iterator it;
            it = compImages.find(resource);
            if (it != compImages.end())
                return it->second;
            return -1;

        }
        bool Scene::addEntity(Entity *entity) {
            if (entity->isAnimated()) {
                if (entity->getCurrentFrame() != nullptr) {
                    //std::cout<<"position of current frame : "<<entity->getCurrentFrame()->getPosition()<<std::endl;
                    addEntity(entity->getCurrentFrame());
                }
            } else {
                std::vector<Entity*> children = entity->getChildren();
                for (unsigned int i = 0; i < children.size(); i++) {
                     addEntity(children[i]);
                }
            }
            for (unsigned int j = 0; j < entity->getFaces().size(); j++) {
                 if (entity->getFace(j)->getMaterial().getTexture() != nullptr) {
                     increaseComptImg(entity->getFace(j)->getMaterial().getTexture());
                 }
            }
            return gridMap->addEntity(entity);
        }
        bool Scene::removeEntity (Entity *entity) {
            if (entity != nullptr) {

                if (entity->isAnimated()) {
                    if (entity->getCurrentFrame() != nullptr) {
                        removeEntity(entity->getCurrentFrame());
                    }
                } else {
                    std::vector<Entity*> children = entity->getChildren();
                    for (unsigned int i = 0; i < children.size(); i++) {
                        removeEntity(children[i]);
                    }
                }
                std::vector<Face> faces = entity->getFaces();
                for (unsigned int j = 0; j < faces.size(); j++) {
                    decreaseComptImg(faces[j].getMaterial().getTexture());
                }
                if(!gridMap->removeEntity(entity)) {
                   return false;
                }
            }
            return true;
        }
        bool Scene::deleteEntity (Entity *entity) {
            if (entity->isAnimated()) {
                if (entity->getCurrentFrame() != nullptr) {
                    deleteEntity(entity->getCurrentFrame());
                }
            } else {
                std::vector<Entity*> children = entity->getChildren();
                for (unsigned int i = 0; i < children.size(); i++) {
                    deleteEntity(children[i]);
                }
            }
            if (entity->getParent() != nullptr) {
                //std::cout<<"remove entity : "<<entity->getType()<<std::endl;
                gridMap->removeEntity(entity);
            }
            std::vector<Face> faces = entity->getFaces();
            for (unsigned int j = 0; j < faces.size(); j++) {
                decreaseComptImg(faces[j].getMaterial().getTexture());
            }
            if (entity->getParent() == nullptr && !gridMap->deleteEntity(entity)) {
                return false;
            }
            return true;
        }
        math::Vec3f Scene::getPosition() {
            return gridMap->getMins();
        }
        int Scene::getWidth() {
            return gridMap->getSize().x;
        }
        int Scene::getHeight() {
            return gridMap->getSize().y;
        }
        int Scene::getNbCasesPerRow () {
            return gridMap->getNbCasesPerRow();
        }
        bool Scene::removeEntity(int id) {
            return gridMap->deleteEntity(id);
        }
        void Scene::rotateEntity(Entity *entity, int angle) {
            removeEntity(entity);
            entity->setRotation(angle);
            addEntity(entity);
        }
        void Scene::scaleEntity(Entity *entity, float sx, float sy) {
            removeEntity(entity);
            entity->setScale(math::Vec3f(sx, sy, 0));
            addEntity(entity);
        }
        void Scene::moveEntity(Entity *entity, float dx, float dy, float dz) {
            removeEntity(entity);
            entity->move(math::Vec3f(dx, dy, dz));
            addEntity(entity);
        }
        void Scene::checkVisibleEntities(EntityFactory& factory) {
            for (unsigned int c = 0; c < frcm->getNbComponents() + 1; c++) {
                if (c == frcm->getNbComponents() || c < frcm->getNbComponents() && frcm->getRenderComponent(c) != nullptr) {
                    physic::BoundingBox view;
                    if (c == frcm->getNbComponents())
                        view = frcm->getWindow().getView().getViewVolume();
                    else
                        view = frcm->getRenderComponent(c)->getView().getViewVolume();
                    visibleEntities.clear();
                    //visibleEntities.resize(core::Application::app->getNbEntitiesTypes());
                    visibleEntities.resize(factory.getNbEntitiesTypes());
                    for (unsigned int i = 0; i < visibleEntities.size(); i++) {
                        //visibleEntities[i].resize(core::Application::app->getNbEntities(), nullptr);
                        visibleEntities[i].resize(factory.getNbEntities(), nullptr);
                    }
                    int x = view.getPosition().x;
                    int y = view.getPosition().y;
                    int z = view.getPosition().z;
                    int endX = view.getPosition().x + view.getWidth();
                    int endY = view.getPosition().y + view.getHeight()+100;
                    int endZ = (gridMap->getCellDepth() > 0) ? view.getPosition().z + view.getDepth()+100 : z;
                    physic::BoundingBox bx (x, y, z, endX-view.getPosition().x, endY-view.getPosition().y, endZ-view.getPosition().z);

                    for (int i = x; i <= endX; i+=gridMap->getOffsetX()) {
                        for (int j = y; j <= endY; j+=gridMap->getOffsetY()) {
                            for (int k = z; k <= endZ; k+=gridMap->getOffsetZ()) {
                                math::Vec3f point(i, j, k);
                                CellMap* cell = getGridCellAt(point);
                                if (cell != nullptr) {
                                    for (unsigned int n = 0; n < cell->getNbEntitiesInside(); n++) {
                                       Entity* entity = cell->getEntityInside(n);
                                       if (visibleEntities[entity->getRootTypeInt()][entity->getId()] == nullptr) {
                                           /*if (entity->getRootType() == "E_MONSTER" && entity->getFaces().size() > 0)
                                               std::cout<<"map tex coords : "<<entity->getFace(0)->getVertexArray()[0].texCoords.x<<","<<entity->getFace(0)->getVertexArray()[0].texCoords.y<<std::endl;*/
                                           visibleEntities[entity->getRootTypeInt()][entity->getId()] = entity;
                                       }
                                    }
                                }
                            }
                        }
                    }
                }
                if (c < frcm->getNbComponents() && frcm->getRenderComponent(c) != nullptr) {
                    std::vector<Entity*> entities = getVisibleEntities(frcm->getRenderComponent(c)->getExpression(), factory);
                    frcm->getRenderComponent(c)->loadEntitiesOnComponent(entities);
                }
            }
        }
        void Scene::setBaseChangementMatrix (BaseChangementMatrix bm) {
            gridMap->setBaseChangementMatrix(bm);
        }
        Entity* Scene::getEntity(int id) {
            return gridMap->getEntity(id);
        }
        Entity* Scene::getEntity(std::string name) {
            return gridMap->getEntity(name);
        }
        vector<CellMap*> Scene::getCasesMap() {
            return gridMap->getCasesMap();
        }

        void Scene::getChildren (Entity *entity, std::vector<Entity*>& entities, std::string type) {
            vector<Entity*> children = entity->getChildren();
            for (unsigned int i = 0; i < children.size(); i++) {
                if (children[i]->getChildren().size() != 0)
                    getChildren(children[i], children, type);
                if (type.size() > 0 && type.at(0) == '*') {
                    std::string types;
                    if (type.find("-") != string::npos)
                        types = type.substr(2, type.size() - 3);
                    vector<string> excl = core::split(types, "-");
                    bool exclude = false;
                    for (unsigned int j = 0; j < excl.size(); j++) {
                        if (children[i]->getType() == excl[j])
                            exclude = true;
                    }
                    if (!exclude) {
                        entities.push_back(children[i]);
                    }
                } else {
                   vector<string> types = core::split(type, "+");
                   for (unsigned int j = 0; j < types.size(); j++) {
                        if (children[i]->getType() == types[j]) {
                            entities.push_back(children[i]);
                        }
                   }
                }
            }
        }

        vector<Entity*> Scene::getEntities(string type) {
            vector<Entity*> entities;
            vector<Entity*> allEntities = gridMap->getEntities();
            if (type.size() > 0 && type.at(0) == '*') {
                if (type.find("-") != string::npos)
                    type = type.substr(2, type.size() - 3);
                vector<string> excl = core::split(type, "-");
                for (unsigned int i = 0; i < allEntities.size(); i++) {
                    Entity* entity = allEntities[i];
                    bool exclude = false;
                    for (unsigned int j = 0; j < excl.size(); j++) {
                        if (entity->getType() == excl[j])
                            exclude = true;
                    }
                    if (!exclude) {
                        bool contains = false;
                        for (unsigned int n = 0; n < entities.size() && !contains; n++) {
                            if (entities[n] == entity) {
                                contains = true;
                            }
                        }
                        if (!contains) {
                            entity->updateTransform();
                            entities.push_back(entity);
                        }
                    }
                }
                return entities;
            }
            vector<string> types = core::split(type, "+");
            for (unsigned int i = 0; i < types.size(); i++) {
                for (unsigned int j = 0; j < allEntities.size(); j++) {
                    Entity* entity = allEntities[j];
                    if (entity->getType() == types[i]) {
                        bool contains = false;
                        for (unsigned int n = 0; n < entities.size() && !contains; n++) {
                            if (entities[n] == entity) {
                                contains = true;
                            }
                        }
                        if (!contains) {
                            entity->updateTransform();
                            entities.push_back(entity);
                        }
                    }
                }
            }
            return entities;
        }
        vector<Entity*> Scene::getRootEntities(string type) {
            vector<Entity*> entities;
            vector<Entity*> allEntities = gridMap->getEntities();
            if (type.size() > 0 && type.at(0) == '*') {
                if (type.find("-") != string::npos)
                    type = type.substr(2, type.size() - 3);
                vector<string> excl = core::split(type, "-");
                for (unsigned int i = 0; i < allEntities.size(); i++) {
                    Entity* entity = allEntities[i]->getRootEntity();
                    bool exclude = false;
                    for (unsigned int j = 0; j < excl.size(); j++) {
                        if (entity->getRootType() == excl[j])
                            exclude = true;
                    }
                    if (!exclude) {
                        bool contains = false;
                        for (unsigned int n = 0; n < entities.size() && !contains; n++) {
                            if (entities[n]->getRootEntity() == entity->getRootEntity()) {
                                contains = true;
                            }
                        }
                        if (!contains) {
                            entity->getRootEntity()->updateTransform();
                            entities.push_back(entity->getRootEntity());
                        }
                    }
                }
                return entities;
            }
            vector<string> types = core::split(type, "+");
            for (unsigned int i = 0; i < types.size(); i++) {
                for (unsigned int j = 0; j < allEntities.size(); j++) {
                    Entity* entity = allEntities[j]->getRootEntity();
                    if (entity->getRootType() == types[i]) {
                        bool contains = false;
                        for (unsigned int n = 0; n < entities.size() && !contains; n++) {
                            if (entities[n]->getRootEntity() == entity->getRootEntity()) {
                                contains = true;
                            }
                        }
                        if (!contains) {
                            entity->getRootEntity()->updateTransform();
                            entities.push_back(entity->getRootEntity());
                        }
                    }
                }
            }
            return entities;
        }
        vector<Entity*> Scene::getChildrenEntities(string type) {
            vector<Entity*> entities;
            vector<Entity*> allEntities = gridMap->getEntities();
            if (type.size() > 0 && type.at(0) == '*') {
                if (type.find("-") != string::npos)
                    type = type.substr(2, type.size() - 3);
                vector<string> excl = core::split(type, "-");
                for (unsigned int i = 0; i < allEntities.size(); i++) {
                    Entity* entity = allEntities[i];
                    bool exclude = false;
                    for (unsigned int j = 0; j < excl.size(); j++) {
                        if (entity->getRootType() == excl[i])
                            exclude = true;
                    }
                    if (!exclude) {
                        bool contains = false;
                        for (unsigned int n = 0; n < entities.size() && !contains; n++) {
                            if (entities[n] == entity) {
                                contains = true;
                            }
                        }
                        if (!contains) {
                            entity->getRootEntity()->updateTransform();
                            entities.push_back(entity);
                        }
                    }
                }
                return entities;
            }
            vector<string> types = core::split(type, "+");
            for (unsigned int i = 0; i < types.size(); i++) {
                for (unsigned int j = 0; j < allEntities.size(); j++) {
                    Entity* entity = allEntities[j];
                    if (entity->getRootType() == types[i]) {
                        bool contains = false;
                        for (unsigned int n = 0; n < entities.size() && !contains; n++) {
                            if (entities[n] == entity) {
                                contains = true;
                            }
                        }
                        if (!contains) {
                            entity->getRootEntity()->updateTransform();
                            entities.push_back(entity);
                        }
                    }
                }
            }
            return entities;
        }

        vector<Entity*> Scene::getVisibleEntities (std::string type, EntityFactory& factory) {
            std::vector<Entity*> entities;
            if (type.size() > 0 && type.at(0) == '*') {
                if (type.find("-") != string::npos)
                    type = type.substr(2, type.size() - 2);
                vector<string> excl = core::split(type, "-");
                for (unsigned int i = 0; i < visibleEntities.size(); i++) {
                    for (unsigned int j = 0; j < visibleEntities[i].size(); j++) {
                        if (visibleEntities[i][j] != nullptr) {
                            bool exclude = false;
                            for (unsigned int t = 0; t < excl.size(); t++) {
                                if (visibleEntities[i][j]->getRootType() == excl[t])
                                    exclude = true;
                            }
                            if (!exclude) {
                                Entity* ba = visibleEntities[i][j]->getRootEntity();
                                if (ba->getBoneAnimationIndex() == visibleEntities[i][j]->getBoneIndex()) {
                                    entities.push_back(visibleEntities[i][j]);
                                }
                            }
                        }
                    }
                }
                return entities;
            }
            vector<string> types = core::split(type, "+");
            for (unsigned int t = 0; t < types.size(); t++) {
                //unsigned int type = core::Application::app->getIntOfType(types[t]);
                unsigned int type = factory.getIntOfType(types[t]);

                if (type < visibleEntities.size()) {
                    vector<Entity*> visibleEntitiesType = visibleEntities[type];
                    for (unsigned int i = 0; i < visibleEntitiesType.size(); i++) {
                        bool found = false;
                        for (unsigned int j = 0; j < types.size(); j++) {

                            if (visibleEntitiesType[i] != nullptr && visibleEntitiesType[i]->getRootType() == types[j]) {
                                found = true;
                            }
                        }
                        if (visibleEntitiesType[i] != nullptr && found) {
                            Entity* ba = visibleEntitiesType[i]->getRootEntity();
                            if (ba->getBoneAnimationIndex() == visibleEntitiesType[i]->getBoneIndex()) {
                                /*if (visibleEntitiesType[i]->getRootType() == "E_MONSTER" && visibleEntitiesType[i]->getFaces().size() > 0)
                                        std::cout<<"bone index : "<<ba->getBoneAnimationIndex()<<" get visible entities texCoords : "<<visibleEntitiesType[i]->getFace(0)->getVertexArray()[0].texCoords.x<<","<<visibleEntitiesType[i]->getFace(0)->getVertexArray()[0].texCoords.y<<std::endl;*/
                                entities.push_back(visibleEntitiesType[i]);
                            }
                        }
                    }
                }
            }
            return entities;
        }

        vector<Entity*> Scene::getEntitiesInBox (physic::BoundingBox bx, std::string type) {
             vector<Entity*> entities;
             vector<Entity*> allEntitiesInRect = gridMap->getEntitiesInBox(bx);
             std::cout<<"entities in rect :"<<allEntitiesInRect.size()<<std::endl;

             if (type.at(0) == '*') {
                if (type.find("-") != string::npos)
                    type = type.substr(2, type.size() - 3);
                vector<string> excl = core::split(type, "-");
                for (unsigned int i = 0; i < allEntitiesInRect.size(); i++) {
                    Entity* entity = allEntitiesInRect[i];
                    if (entity != nullptr) {
                        bool exclude = false;
                        for (unsigned int i = 0; i < excl.size(); i++) {
                            if (entity->getRootType() == excl[i])
                                exclude = true;
                        }
                        if (!exclude) {
                            Entity* ba = entity->getRootEntity();
                            if (ba->getBoneAnimationIndex() == entity->getBoneIndex()) {
                                entities.push_back(entity);
                            }
                        }
                    }
                }
                return entities;
            }
            vector<string> types = core::split(type, "+");
            for (unsigned int i = 0; i < types.size(); i++) {
                for (unsigned int j = 0; j < allEntitiesInRect.size(); j++) {
                    Entity* entity = allEntitiesInRect[j];
                    if (entity != nullptr) {
                        if (entity->getRootType() == types[i]) {
                            Entity* ba = entity->getRootEntity();
                            if (ba->getBoneAnimationIndex() == entity->getBoneIndex()) {
                                entities.push_back(entity);
                            }
                        }
                    }
                }
            }
            return entities;
        }
        math::Vec3f Scene::getCoordinatesAt(math::Vec3f p) {
            math::Vec3f c(p.x, p.y, p.z);
            return gridMap->getCoordinatesAt(c);
        }
        CellMap* Scene::getGridCellAt(math::Vec3f p) {
            return gridMap->getGridCellAt(p);
        }
        vector<CellMap*> Scene::getCasesInBox (physic::BoundingBox bx) {
            return gridMap->getCasesInBox(bx);
        }
        bool Scene::collide (Entity *entity, math::Vec3f position) {
             return gridMap->collideWithEntity(entity, position);
        }
        bool Scene::collide (Entity *entity) {
             return gridMap->collideWithEntity(entity);
        }
        bool Scene::collide (Entity* entity, math::Ray ray) {
             math::Vec3f point = ray.getOrig() + ray.getDir().normalize() * diagSize * 0.001f;
             math::Vec3f v1 = ray.getExt() - ray.getOrig();
             math::Vec3f v2 = point - ray.getOrig();
             while (v2.magnSquared() / v1.magnSquared() < 1) {
                    if (collide(entity, point))
                        return true;
                    point += ray.getDir().normalize() * diagSize * 0.001f;
                    v2 = point - ray.getOrig();
             }
             point = ray.getExt();
             return collide(entity, point);
        }

        void Scene::drawOnComponents(std::string expression, int layer, sf::BlendMode blendMode) {
            Component* frc = frcm->getRenderComponent(layer);
            if (frc != nullptr) {
                frc->setExpression(expression);
            }
        }
        void Scene::drawOnComponents(Drawable& drawable, int layer, RenderStates states) {
            Component *frc = frcm->getRenderComponent(layer);
            if (frc != nullptr) {
                frc->draw(drawable, states);
            }
        }
        BaseChangementMatrix Scene::getBaseChangementMatrix() {
            return gridMap->getBaseChangementMatrix();
        }
    }
}

