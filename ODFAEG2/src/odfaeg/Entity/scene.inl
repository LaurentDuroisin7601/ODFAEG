#include "scene.hpp"
#include "../../../include/odfaeg/Graphics/rectangleShape.h"
#include "../../../include/odfaeg/Physics/boundingEllipsoid.h"
#include <iostream>
#include <climits>
#include "../../../include/odfaeg/Core/singleton.h"
#include "../../../include/odfaeg/Graphics/tGround.h"
#include "../../../include/odfaeg/Graphics/boneAnimation.hpp"
//#include "../../../include/odfaeg/Graphics/application.h"
namespace odfaeg {
    namespace entity {       
        Scene::Scene (std::string name, int cellWidth, int cellHeight, int cellDepth) : gridMap(cellWidth, cellHeight, cellDepth) {
            
        }
        void Scene::generate_labyrinthe (std::vector<Tile*> tGround, std::vector<Wall*> walls, unsigned int squareSize, physic::BoundingBox &rect) {
            int startX = rect.getPosition().x() / tileSize.x() * tileSize.x();
            int startY = rect.getPosition().z() / tileSize.y() * tileSize.y();
            int endX = (rect.getPosition().x() + rect.getWidth()) / tileSize.x() * tileSize.x();
            int endY = (rect.getPosition().z() + rect.getHeight()) / tileSize.y() * tileSize.y();
            HeightMap hm = new HeightMap(squareSize, rect);
            hp->setSize(rect.getSize());
            unsigned int i, j;
            //Génération du sol et de tout les murs.
            for (int y = startY, j = 0; y < endY; y+= tileSize.y(), j++) {
                for (int x = startX, i = 0; x < endX; x+= tileSize.x(), i++) {
                    math::Vec3f projPos = baseChangmentMatrix.changeOfBase(math::Vec3f (x - startX, y - startY, 0));
                    math::Vec2f pos (projPos.x() + startX, projPos.y() + startY);
                    if (x == startX && y == startY) {
                        Entity *w = walls[Wall::TOP_LEFT]->clone();
                        w->setPosition(math::Vec3f(pos.x(), rect.getPosition().z(),pos.y()));
                        w->setSize(math::Vec3f(w->getSize().x(), rect.getSize().y() + w->getSize().y(), w->getSize().z()));
                        addEntity(w);
                        gridMap.getGridCellAt(math::Vec3f(w->getPosition().x(), rect.getPosition().y(), w->getPosition().y()))->setPassable(false);
                    } else if (x == endX && y == startY) {
                        Entity *w = walls[Wall::TOP_RIGHT]->clone();
                        w->setPosition(math::Vec3f(pos.x(), rect.getPosition().z(),pos.y()));
                        w->setSize(math::Vec3f(w->getSize().x(), rect.getSize().y() + w->getSize().y(), w->getSize().z()));
                        addEntity(w);
                        gridMap.getGridCellAt(math::Vec3f(w->getPosition().x(), rect.getPosition().y(), w->getPosition().y()))->setPassable(false);
                    } else if (y == endY && x == endX) {
                        Entity *w = walls[Wall::BOTTOM_RIGHT]->clone();
                        w->setPosition(math::Vec3f(pos.x(), rect.getPosition().z(),pos.y()));
                        w->setSize(math::Vec3f(w->getSize().x(), rect.getSize().y() + w->getSize().y(), w->getSize().z()));
                        addEntity(w);
                        gridMapgetGridCellAt(math::Vec3f(w->getPosition().x(), rect.getPosition().y(), w->getPosition().y()))->setPassable(false);
                    } else if (x == startX && y == endY) {
                        Entity *w = walls[Wall::BOTTOM_LEFT]->clone();
                        w->setPosition(math::Vec3f(pos.x(), rect.getPosition().z(),pos.y()));
                        w->setSize(math::Vec3f(w->getSize().x(), rect.getSize().y() + w->getSize().y(), w->getSize().z()));
                        addEntity(w);
                        gridMap.getGridCellAt(math::Vec3f(w->getPosition().x(), rect.getPosition().y(), w->getPosition().y()))->setPassable(false);
                    } else if (y == startY && j % 2 != 0) {
                        Entity *w = walls[Wall::TOP_BOTTOM]->clone();
                        w->setPosition(math::Vec3f(pos.x(), rect.getPosition().z(),pos.y()));
                        w->setSize(math::Vec3f(w->getSize().x(), rect.getSize().y() + w->getSize().y(), w->getSize().z()));
                        addEntity(w);
                        gridMap.getGridCellAt(math::Vec3f(w->getPosition().x(), rect.getPosition().y(), w->getPosition().y()))->setPassable(false);
                    } else if (y == startY && j % 2 == 0) {
                        Entity *w = walls[Wall::T_TOP]->clone();
                        w->setPosition(math::Vec3f(pos.x(), rect.getPosition().z(),pos.y()));
                        w->setSize(math::Vec3f(w->getSize().x(), rect.getSize().y() + w->getSize().y(), w->getSize().z()));
                        addEntity(w);
                        gridMap.getGridCellAt(math::Vec3f(w->getPosition().x(), rect.getPosition().y(), w->getPosition().y()))->setPassable(false);
                    } else if (x == endX && j % 2 != 0) {
                        Entity *w = walls[Wall::RIGHT_LEFT]->clone();
                        w->setPosition(math::Vec3f(pos.x(), rect.getPosition().z(),pos.y()));
                        w->setSize(math::Vec3f(w->getSize().x(), rect.getSize().y() + w->getSize().y(), w->getSize().z()));
                        addEntity(w);
                        gridMap.getGridCellAt(math::Vec3f(w->getPosition().x(), rect.getPosition().y(), w->getPosition().y()))->setPassable(false);
                    } else if (x == endX && j % 2 == 0) {
                        Entity *w = walls[Wall::T_RIGHT]->clone();
                        w->setPosition(math::Vec3f(pos.x(), rect.getPosition().z(),pos.y()));
                        w->setSize(math::Vec3f(w->getSize().x(), rect.getSize().y() + w->getSize().y(), w->getSize().z()));
                        addEntity(w);
                        gridMap.getGridCellAt(math::Vec3f(w->getPosition().x(), rect.getPosition().y(), w->getPosition().y()))->setPassable(false);
                    } else if(y == endY && i % 2 != 0) {
                        Entity *w = walls[Wall::TOP_BOTTOM]->clone();
                        w->setPosition(math::Vec3f(pos.x(), rect.getPosition().z(),pos.y()));
                        w->setSize(math::Vec3f(w->getSize().x(), rect.getSize().y() + w->getSize().y(), w->getSize().z()));
                        addEntity(w);
                        gridMap.getGridCellAt(math::Vec3f(w->getPosition().x(), rect.getPosition().y(), w->getPosition().y()))->setPassable(false);
                    } else if (y == endY && i % 2 == 0) {
                        Entity *w = walls[Wall::T_BOTTOM]->clone();
                        w->setPosition(math::Vec3f(pos.x(), rect.getPosition().z(),pos.y()));
                        w->setSize(math::Vec3f(w->getSize().x(), rect.getSize().y() + w->getSize().y(), w->getSize().z()));
                        addEntity(w);
                        gridMap.getGridCellAt(math::Vec3f(w->getPosition().x(), rect.getPosition().y(), w->getPosition().y()))->setPassable(false);
                    } else if (x == startX && j % 2 != 0) {
                        Entity *w = walls[Wall::RIGHT_LEFT]->clone();
                        w->setPosition(math::Vec3f(pos.x(), rect.getPosition().z(),pos.y()));
                        w->setSize(math::Vec3f(w->getSize().x(), rect.getSize().y() + w->getSize().y(), w->getSize().z()));
                        addEntity(w);
                        gridMap.getGridCellAt(math::Vec3f(w->getPosition().x(), rect.getPosition().y(), w->getPosition().y()))->setPassable(false);
                    } else if (x == startX && j % 2 == 0) {
                        Entity *w = walls[Wall::T_LEFT]->clone();
                        w->setPosition(math::Vec3f(pos.x(), rect.getPosition().z(),pos.y()));
                        w->setSize(math::Vec3f(w->getSize().x(), rect.getSize().y() + w->getSize().y(), w->getSize().z()));
                        addEntity(w);
                        gridMap.getGridCellAt(math::Vec3f(w->getPosition().x(), rect.getPosition().y(), w->getPosition().y()))->setPassable(false);
                    } else if (j % 2 != 0 && i % 2 == 0) {
                        Entity *w = walls[Wall::RIGHT_LEFT]->clone();
                        w->setPosition(math::Vec3f(pos.x(), rect.getPosition().z(),pos.y()));
                        w->setSize(math::Vec3f(w->getSize().x(), rect.getSize().y() + w->getSize().y(), w->getSize().z()));
                        addEntity(w);
                        gridMap.getGridCellAt(math::Vec3f(w->getPosition().x(), rect.getPosition().y(), w->getPosition().y()))->setPassable(false);
                    } else if (j % 2 == 0 && i % 2 != 0) {
                        Entity *w = walls[Wall::TOP_BOTTOM]->clone();
                        w->setPosition(math::Vec3f(pos.x(), rect.getPosition().z(),pos.y()));
                        w->setSize(math::Vec3f(w->getSize().x(), rect.getSize().y() + w->getSize().y(), w->getSize().z()));
                        addEntity(w);
                        gridMap.getGridCellAt(math::Vec3f(w->getPosition().x(), rect.getPosition().y(), w->getPosition().y()))->setPassable(false);
                    } else if (j % 2 == 0 && i % 2 == 0) {
                        Entity *w = walls[Wall::X]->clone();
                        w->setPosition(math::Vec3f(pos.x(), rect.getPosition().z(),pos.y()));
                        w->setSize(math::Vec3f(w->getSize().x(), rect.getSize().y() + w->getSize().y(), w->getSize().z()));
                        addEntity(w);
                        gridMap.getGridCellAt(math::Vec3f(w->getPosition().x(), rect.getPosition().y(), w->getPosition().y()))->setPassable(false);
                    } else {
                        Entity* tile;
                        if (tGround.size() > 0)  {
                            int i = math::Math::random(tGround.size());
                            tile = tGround[i]->clone();
                            tile->setPosition(math::Vec3f(pos.x(), 0, pos.y()));
                            //////////std::cout<<"add tile : "<<tile->getPosition()<<std::endl;
                        } else {
                            tile = factory.make_entity<Tile>(nullptr, math::Vec3f(pos.x(), 0, pos.y()), math::Vec3f(tileSize.x(), 0, tileSize.y()), IntRect(0, 0, tileSize.x(), tileSize.y()), factory);
                        }
                        float heights[4];
                        for (unsigned int j = 0; j < sizeof(heights) / sizeof(float); j++) {
                            heights[j] = math::Math::random(rect.getPosition().y(), rect.getPosition().y() + rect.getHeight());
                        }
                        tile->changeVerticesHeights(heights[0], heights[1], heights[2], heights[3]);
                        hm->addSquare(tile);
                    }
            }
            addEntity(hm);
            //Génération du labyrinthe.
            std::vector<CellMap*> visited;
            std::vector<math::Vec2f> dirs;
            int n = 0, cx = startX + squareSize * 0.5f, cy = startY + squareSize * 0.5f;
            //boucle tant que toutes les cellules n'ont pas été visitées.
            while (n < i * j) {
                //directions possibles pour avancer dans le labyrinthe.
                dirs.clear();
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
                    x += dir.x() * squareSize;
                    y += dir.y() * squareSize;
                    math::Vec3f projPos = baseChangmentMatrix.changeOfBase(math::Vec3f (x - startX, y - startY, 0));
                    math::Vec2f pos (projPos.x() + startX, projPos.y() + startY);
                    CellMap* cell = gridMap->getGridCellAt(math::Vec3f(pos.x(), rect.getPosition().y(), pos.y()));
                    if (cell != nullptr) {
                        dirs.push_back(dir);
                    }
                }
                //Parcours d'un chemin aléatoire.
                math::Vec2f dir = dirs[math::Math::random(0, dirs.size())];
                int x = cx + dir.x() * squareSize;
                int y = cy + dir.y() * squareSize;
                math::Vec3f projPos = baseChangmentMatrix.changeOfBase(math::Vec3f (x - startX, y - startY, 0));
                math::Vec2f pos (projPos.x() + startX, projPos.y() + startY);
                //Suppression du mur entre les deux cellules.
                CellMap* cell = gridMap->getGridCellAt(math::Vec3f(pos.x(), rect.getPosition().y(), pos.y()));
                std::vector<Entity*> wall = cell->getEntitiesInside("E_WALL");
                for (unsigned int i = 0; i < wall.size(); i++) {
                    deleteEntity(wall[i]);
                }
                cell->setPassable(true);
                //On avance de deux cellules.
                cx += dir.x() * squareSize * 2;
                cy += dir.y() * squareSize * 2;
                x = cx, y = cy;
                projPos = baseChangmentMatrix.changeOfBase(math::Vec3f (x - startX, y - startY, 0));
                pos = math::Vec2f (projPos.x() + startX, projPos.y() + startY);
                cell = gridMap->getGridCellAt(math::Vec3f(pos.x(), rect.getPosition().y(), pos.y()));
                bool contains = false;
                //Marque cette cellule comme visitée.
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
        void Scene::generate_arena(std::vector<Tile*> tGround, std::vector<g3d::Wall*> walls, unsigned int squareSize, physic::BoundingBox &rect, EntityFactory& factory) {
            int startX = rect.getPosition().x() / tileSize.x() * tileSize.x();
            int startY = rect.getPosition().z() / tileSize.y() * tileSize.y();
            int endX = (rect.getPosition().x() + rect.getWidth()) / tileSize.x() * tileSize.x();
            int endY = (rect.getPosition().z() + rect.getDepth()) / tileSize.y() * tileSize.y();
            HeightMap *hp = factory.make_entity<BigTile>(math::Vec3f(startX, rect.getPosition().y(), startY),factory, tileSize,rect.getWidth() / tileSize.x());
            hp->setSize(rect.getSize());
            //bt->setCenter(math::Vec3f(rect.getCenter().x, rect.getCenter().y, rect.getPosition().z()));
            //Positions de d\E9part et d'arriv\E9es en fonction de la taille, de la position et de la taille des cellules de la map.
            for (int y = startY; y < endY;  y+=tileSize.y()) {
                for (int x = startX; x < endX; x+=tileSize.x()) {
                    ////////std::cout<<"start x y : "<<startX<<","<<startY<<std::endl;
                    ////////std::cout<<"end x y : "<<endX-tileSize.x()<<","<<endY-tileSize.y()<<std::endl;
                    ////////std::cout<<"x y : "<<x<<","<<y<<std::endl;
                    //On projete les positions en fonction de la projection du jeux.
                    math::Vec3f projPos = baseChangementMatrix.changeOfBase(math::Vec3f (x - startX, y - startY, 0));
                    math::Vec2f pos (projPos.x() + startX, projPos.y() + startY);
                    //////////std::cout<<"pos : "<<pos<<std::endl;
                    //Mur du coin en haut \E0 gauche.
                    if (x == startX && y == startY && walls.size() >= 11) {
                        if (walls[Wall::TOP_LEFT] != nullptr) {
                            ////////std::cout<<"top left"<<std::endl;
                            Entity *w = walls[Wall::TOP_LEFT]->clone();
                            w->setPosition(math::Vec3f(pos.x(), rect.getPosition().y(), pos.y()));
                            w->setSize(math::Vec3f(w->getSize().x(), rect.getSize().z() + w->getSize().z(), w->getSize().z()));
                            //////////std::cout<<"position top right : "<<w->getPosition()<<std::endl;
                            addEntity(w);
                            gridMap.getGridCellAt(math::Vec3f(w->getPosition().x(), w->getPosition().y(), w->getPosition().z()))->setPassable(false);
                        }

                        //Mur du coin en haut \E0 droite.
                    } else if (x == endX - tileSize.x() && y == startY && walls.size() >= 11) {
                        if (walls[Wall::TOP_RIGHT] != nullptr) {
                            ////////std::cout<<"top right"<<std::endl;
                            Entity *w = walls[Wall::TOP_RIGHT]->clone();
                            w->setPosition(math::Vec3f(pos.x(), rect.getPosition().y(), pos.y()));
                            w->setSize(math::Vec3f(w->getSize().x(), rect.getSize().z() + w->getSize().z(), w->getSize().z()));
                            //////////std::cout<<"position top right : "<<w->getPosition()<<std::endl;
                            addEntity(w);
                            gridMap.getGridCellAt(math::Vec3f(w->getPosition().x(), w->getPosition().y(), w->getPosition().z()))->setPassable(false);
                        }
                        //Mur du coin en bas \E0 droite.
                    } else if (x == endX - tileSize.x() && y == endY - tileSize.y() && walls.size() >= 11) {
                        if (walls[Wall::BOTTOM_RIGHT] != nullptr) {
                            ////////std::cout<<"bottom right"<<std::endl;
                            Entity *w = walls[Wall::BOTTOM_RIGHT]->clone();
                            w->setPosition(math::Vec3f(pos.x(), rect.getPosition().y(), pos.y()));
                            w->setSize(math::Vec3f(w->getSize().x(), rect.getSize().z() + w->getSize().z(), w->getSize().z()));
                            addEntity(w);
                            gridMap.getGridCellAt(math::Vec3f(w->getPosition().x(), w->getPosition().y(), w->getPosition().z()))->setPassable(false);
                        }
                    } else if (x == startX && y == endY - tileSize.y() && walls.size() >= 11) {
                        if (walls[Wall::BOTTOM_LEFT] != nullptr) {
                            ////////std::cout<<"bottom left"<<std::endl;
                            Entity *w = walls[Wall::BOTTOM_LEFT]->clone();
                            w->setPosition(math::Vec3f(pos.x(), rect.getPosition().y(), pos.y()));
                            w->setSize(math::Vec3f(w->getSize().x(), rect.getSize().z() + w->getSize().z(), w->getSize().z()));
                            //////////std::cout<<"position bottom left : "<<w->getPosition()<<std::endl;
                            addEntity(w);
                            gridMap.getGridCellAt(math::Vec3f(w->getPosition().x(), w->getPosition().y(), w->getPosition().z()))->setPassable(false);
                        }
                    } else if ((y == startY || y == endY - tileSize.y()) && walls.size() >= 11) {
                        if (walls[Wall::TOP_BOTTOM] != nullptr) {
                            ////////std::cout<<"top bottom"<<std::endl;
                            Entity *w = walls[Wall::TOP_BOTTOM]->clone();
                            w->setPosition(math::Vec3f(pos.x(), rect.getPosition().y(), pos.y()));
                            w->setSize(math::Vec3f(w->getSize().x(), rect.getSize().z() + w->getSize().z(), w->getSize().z()));
                            addEntity(w);
                            /*if (y == endY - tileSize.y()) {
                                int i = math::Math::random(tGround.size());
                                Entity *tile = tGround[i]->clone();
                                tile->setPosition(math::Vec3f(pos.x(), 0, pos.y()));
                                bt->addTile(tile);
                            }*/
                            gridMap.getGridCellAt(math::Vec3f(w->getPosition().x(), w->getPosition().y(), w->getPosition().z()))->setPassable(false);
                        }
                    } else if ((x == startX || x == endX - tileSize.x()) && walls.size() >= 11) {
                        if (walls[Wall::RIGHT_LEFT] != nullptr) {
                            ////////std::cout<<"right left"<<std::endl;
                            Entity *w = walls[Wall::RIGHT_LEFT]->clone();
                            w->setPosition(math::Vec3f(pos.x(), rect.getPosition().y(), pos.y()));
                            w->setSize(math::Vec3f(w->getSize().x(), rect.getSize().z() + w->getSize().z(), w->getSize().z()));
                            addEntity(w);
                            /*if (x == endX - tileSize.x()) {
                                int i = math::Math::random(tGround.size());
                                Entity *tile = tGround[i]->clone();
                                tile->setPosition(math::Vec3f(pos.x(), 0, pos.y()));
                                bt->addTile(tile);
                            }*/
                            gridMap->getGridCellAt(math::Vec3f(w->getPosition().x(), w->getPosition().y(), w->getPosition().z()))->setPassable(false);
                        }
                    }
                    Entity* tile;
                    if (tGround.size() > 0)  {
                        int i = math::Math::random(tGround.size());
                        tile = tGround[i]->clone();
                        tile->setPosition(math::Vec3f(pos.x(), 0, pos.y()));
                    } else {
                        tile = new Tile(nullptr, math::Vec3f(pos.x(), 0, pos.y()), math::Vec3f(squareSize, 0, squareSize.y()), IntRect(0, 0, 1, 1));
                    }
                    float heights[4];
                    for (unsigned int j = 0; j < 4; j++) {
                        heights[j] = math::Math::random(rect.getPosition().y(), rect.getPosition().y() + rect.getHeight());
                    }
                    tile->changeVerticesHeights(heights[0], heights[1], heights[2], heights[3]);
                    hm->addTile(tile);
                }
            }
            addEntity(bm);
        }       
        void Scene::setName (string name) {
            this->name = name;
        }
        string Scene::getName() {
            return name;
        }
        void Scene::removeComptImg (const void* resource) {
            map<const void*, int>::iterator it;
            it = compImages.find(resource);
            if (it != compImages.end()) {
                compImages.erase(it);
            }
        }
        void Scene::increaseComptImg(std::string resource) {
            map<std::string, int>::iterator it;
            it = compResources.find(resource);
            if (it != compResources.end()) {
                it->second = it->second + 1;
            } else {
                compResources.insert(pair<std::string, int> (resource, 1));
            }
        }
        void Scene::decreaseComptImg (std::string resource) {
            map<std::string, int>::iterator it;
            it = compResources.find(resource);
            if (it != compResources.end() && it->second != 0) {
                it->second = it->second - 1;
            }
        }
        int Scene::getCompImage(std::string resource) {
            map<std::string, int>::iterator it;
            it = compResources.find(resource);
            if (it != compResources.end())
                return it->second;
            return -1;
        }
        bool Scene::addGameObject(GameObject *entity) {            
            for (unsigned int j = 0; j < entity->getChildren ().size(); j++) {
                addGameObject(entity->getChildren()[j]);
            }
            for (unsigned int j = 0; j < entity->getSubMesheses().size(); j++) {
               for (unsigned int k = 0; k < entity->getSubMesheses(j)->getAssets().size(); k++) {
                    increaseComptImg(entity->getSubMesheses(j)->getAssets()[k].name);                    
                }
            }
            if (entity->getParent() == nullptr) {                
                std::unique_ptr<Entity> ptr;
                ptr.reset(entity);
                entities.push_back(std::move(ptr));
                ////////std::cout<<"add entity : "<<entity->getType()<<std::endl;
                return gridMap.addEntity(entity);
            }            
        }
        bool Scene::removeEntity (Entity *entity) {
            bool removed = true;
            if (entity != nullptr) {
                std::vector<Entity*> children = entity->getChildren();
                for (unsigned int i = 0; i < children.size(); i++) {
                    removeEntity(children[i]);                    
                }
                              
                for (unsigned int j = 0; j < entity->getSubMesheses().size(); j++) {
                    for (unsigned int k = 0; k < entity->getSubMesheses(j)->getAssets().size(); k++) {
                            decreaseComptImg(entity->getSubMesheses(j)->getAssets()[k].name);                    
                        }
                    }
                }
            }
            if (entity->getParent() == nullptr) {
                ////////std::cout<<"remove entity : "<<entity->getType()<<std::endl;
                if(!gridMap->removeEntity(entity)) {
                   removed = false;
                }  
                std::vector<std::unique_ptr<Entity>>::iterator it;
                for (it = entities.begin(); it != entities.end(); it++) {
                    if (it->get() == entity) {
                        it->release();
                        entities.erase(it);
                        break;
                    }
                }
            }
            return removed;
        }
        bool Scene::deleteEntity (Entity *entity) {
            bool removed = true;
            if (entity != nullptr) {
                std::vector<Entity*> children = entity->getChildren();
                for (unsigned int i = 0; i < children.size(); i++) {
                    removeEntity(children[i]);                    
                }
                ////////std::cout<<"remove entity : "<<entity->getType()<<std::endl;
                if(!gridMap->removeEntity(entity)) {
                   removed = false;
                }                
                for (unsigned int j = 0; j < entity->getSubMesheses().size(); j++) {
                    for (unsigned int k = 0; k < entity->getSubMesheses(j)->getAssets().size(); k++) {
                            decreaseComptImg(entity->getSubMesheses(j)->getAssets()[k].name);                    
                        }
                    }
                }
            }
            if (entity->getParent() == nullptr) {
                ////////std::cout<<"remove entity : "<<entity->getType()<<std::endl;
                if(!gridMap->removeEntity(entity)) {
                   removed = false;
                }  
                std::vector<std::unique_ptr<Entity>>::iterator it;
                for (it = entities.begin(); it != entities.end(); it++) {
                    if (it->get() == entity) {                        
                        entities.erase(it);
                        break;
                    }
                }
            }
            return removed;
        }        
        void Scene::rotateEntity(GameObject *entity, float angle, math::Vec3f axis) {
            removeEntity(entity);
            entity->setRotation(angle, axis);
            addEntity(entity);
        }
        void Scene::scaleEntity(GameObject *entity, float sx, float sy, float sz) {
            removeEntity(entity);
            entity->setScale(math::Vec3f(sx, sy, sz));
            addEntity(entity);
        }
        void Scene::moveEntity(GameObject *entity, float dx, float dy, float dz) {
            removeEntity(entity);
            entity->move(math::Vec3f(dx, dy, dz));
            addEntity(entity);
        }
        void Scene::setBaseChangementMatrix (BaseChangementMatrix bm) {
            baseChangmentMatrix = bm;
        }
        Entity* Scene::getEntity(std::string name) {
            std::vector<GameObject*> allEntities = gridMap.getEntities();
            for (unsigned int i = 0; i < allEntities.size(); i++) {
                Entity* frame;
                if (allEntities[i]->isAnimated()) {
                    frame = checkFrameEntity(allEntities[i], name);
                    if (frame != nullptr) {
                        return frame;
                    }
                } else {
                    if (allEntities[i]->getName() == name)
                        return allEntities[i];
                }
            }
            return nullptr;            
        }        
        vector<Entity*> Scene::getEntities(string expression) {
            vector<Entity*> entities;
            vector<Entity*> allEntities = gridMap->getEntities();
            if (expression.size() > 0 && expression.at(0) == '*') {
                if (expression.find("-") != string::npos)
                    expression = expression.substr(2, expression.size() - 3);
                vector<string> excl = core::split(expression, "-");
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
            vector<string> types = core::split(expression, "+");
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
        vector<Entity*> Scene::getEntitiesInBox (physic::BoundingBox bx, std::string type) {
             vector<Entity*> entities;
             vector<Entity*> allEntitiesInRect = gridMap->getEntitiesInBox(bx);
             ////////std::cout<<"entities in rect :"<<allEntitiesInRect.size()<<std::endl;

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
        bool Scene::collide(Entity *entity, math::Vec3f position) {
             
        }
        bool Scene::collide (Entity *entity) {
             
        }
        bool Scene::collide (Entity* entity, math::Ray ray) {
             math::Vec3f point = ray.getOrig() + ray.getDir().normalize() * diagSize * 0.001f;
             math::Vec3f v1 = ray.getExt() - ray.getOrig();
             math::Vec3f v2 = point - ray.getOrig();
             while (v2.magnSquared() / v1.magnSquared() < 1) {
                    if (collide(entity, point, cinfos))
                        return true;
                    point += ray.getDir().normalize() * diagSize * 0.001f;
                    v2 = point - ray.getOrig();
             }
             point = ray.getExt();
             return collide(entity, point, cinfos);
        }
        math::Vec3f Scene::physicallyBasedComputePos (Entity* entity, math::Vec3f oldCenter, math::Vec3f size, math::Vec3f velocity, float climbCapacity) {
            /*float newFootHeight, newHeadHeight;
            //La nouvelle position du joueur suivant les tests de la physique du jeux.
            math::Vec3f newCenter = entity->getCenter();
            newFootHeight = newCenter.y() - size.y() * 0.5f;
            newHeadHeight = newCenter.y() + size.y() * 0.5f;
            //On cherche d'abord les collisions par rapport aux plateformes.
            //On r�cup�re toutes les plateformes.
            std::vector<Entity*> platforms = getRootEntities("E_BIGTILE");
           
            //Si il y a plusieurs plateformes il faut rechercher sur laquelle le joueur se trouve, c'est � dire la plus proche en dessous.
            float minDist;
            unsigned int index;
            for (unsigned int i = 0; i< platforms.size(); i++) {
                //Hauteur de la plateforme ou le joueur se trouvait.
                float height;
                bool isOnPlateform = platforms[i]->getHeight(math::Vec2f(newCenter.x(), newCenter.y()), height);
                //Distance entre le pied du joueur et la plateforme et la capacit� � monter sur la plateforme.
                float dist = newFootHeight - height;
                //Si le joueur n'est pas en dessous de la plateforme on initialise minDist et on sort de la boucle.
                if (dist <= 0 && isOnPlateform) {
                    minDist = dist;
                    index = i;
                    break;
                }
            }
            Entity* upPlatformFootHeight = nullptr;
            //Il faut refaire une boucle si il y a plusieurs plateforme en dessous du joueur il faut trouver la plus proche.
            for (unsigned int i = index; i < platforms.size(); i++) {
                //Hauteur de la plateforme ou le joueur se trouvait.
                float height;
                bool isOnPlatform = platforms[i]->getHeight(math::Vec2f(newCenter.x(), newCenter.y()), height);
                //Distance entre le pied du joueur et la plateforme et la capacit� � monter sur la pente ou la plateforme.
                float dist = newFootHeight - height;
                //Si le joueur n'est pas en dessous de la plateforme et que la plateforme est la plus proche on initialise la plateforme et la distance minimum et la plateforme.
                if (dist > minDist && dist <= 0 && isOnPlatform) {
                    minDist = dist;
                    upPlatformFootHeight = platforms[i];
                }
            }
            for (unsigned int i = 0; i< platforms.size(); i++) {
                //Hauteur de la plateforme ou le joueur se trouvait.
                float height;
                bool isOnPlatform = platforms[i]->getHeight(math::Vec2f(newCenter.x(), newCenter.y()), height);
                //Distance entre le pied du joueur et la plateforme et la capacit� � monter sur la plateforme.
                float dist = math::Math::abs(height - newHeadHeight);
                if (isOnPlatform) {
                    minDist = dist;
                    index = i;
                    break;
                }
            }
            Entity* upPlatformHeadHeight = nullptr;
            //Il faut refaire une boucle si il y a plusieurs plateforme en dessous du joueur il faut trouver la plus proche.
            for (unsigned int i = index; i < platforms.size(); i++) {
                //Hauteur de la plateforme ou le joueur se trouvait.
                float height;
                bool isOnPlatform = platforms[i]->getHeight(math::Vec2f(newCenter.x(), newCenter.y()), height);
                //Distance entre le pied du joueur et la plateforme et la capacit� � monter sur la pente ou la plateforme.
                float dist = newHeadHeight - height;
                //Si le joueur n'est pas en dessous de la plateforme et que la plateforme est la plus proche on initialise la plateforme et la distance minimum et la plateforme.
                if (dist > minDist && dist <= 0 && isOnPlatform) {
                    minDist = dist;
                    upPlatformHeadHeight = platforms[i];
                }
            }
            for (unsigned int i = 0; i< platforms.size(); i++) {
                //Hauteur de la plateforme ou le joueur se trouvait.
                float height;
                bool isOnPlatform = platforms[i]->getHeight(math::Vec2f(newCenter.x(), newCenter.y()), height);
                //Distance entre le pied du joueur et la plateforme et la capacit� � monter sur la plateforme.
                float dist = math::Math::abs(height - newFootHeight);
                if (isOnPlatform) {
                    minDist = dist;
                    index = i;
                    break;
                }
            }
            Entity* downPlatformFootHeight = nullptr;
            //Il faut refaire une boucle si il y a plusieurs plateforme en dessous du joueur il faut trouver la plus proche.
            for (unsigned int i = index; i < platforms.size(); i++) {
                //Hauteur de la plateforme ou le joueur se trouvait.
                float height;
                bool isOnPlatform = platforms[i]->getHeight(math::Vec2f(newCenter.x(), newCenter.y()), height);
                //Distance entre le pied du joueur et la plateforme et la capacit� � monter sur la pente ou la plateforme.
                float dist = height - newFootHeight;
                //Si le joueur n'est pas en dessous de la plateforme et que la plateforme est la plus proche on initialise la plateforme et la distance minimum et la plateforme.
                if (dist < minDist && dist >= 0 && isOnPlatform) {
                    minDist = dist;
                    downPlatformFootHeight = platforms[i];
                }
            }
            for (unsigned int i = 0; i< platforms.size(); i++) {
                //Hauteur de la plateforme ou le joueur se trouvait.
                float height;
                bool isOnPlatform = platforms[i]->getHeight(math::Vec2f(newCenter.x(), newCenter.y()), height);
                //Distance entre le pied du joueur et la plateforme et la capacit� � monter sur la plateforme.
                float dist = math::Math::abs(height - newHeadHeight);
                if (isOnPlatform) {
                    minDist = dist;
                    index = i;
                    break;
                }
            }
            Entity* downPlatformHeadHeight = nullptr;
            //Il faut refaire une boucle si il y a plusieurs plateforme en dessous du joueur il faut trouver la plus proche.
            for (unsigned int i = index; i < platforms.size(); i++) {
                //Hauteur de la plateforme ou le joueur se trouvait.
                float height;
                bool isOnPlatform = platforms[i]->getHeight(math::Vec2f(newCenter.x(), newCenter.y()), height);
                //Distance entre le pied du joueur et la plateforme et la capacit� � monter sur la pente ou la plateforme.
                float dist = height - newHeadHeight;
                //Si le joueur n'est pas en dessous de la plateforme et que la plateforme est la plus proche on initialise la plateforme et la distance minimum et la plateforme.
                if (dist < minDist && dist >= 0 && isOnPlatform) {
                    minDist = dist;
                    downPlatformHeadHeight = platforms[i];
                }
            }

            if (velocity.y() > 0) {
               
                if (upPlatformHeadHeight != nullptr && downPlatformFootHeight != nullptr &&
                    upPlatformHeadHeight == downPlatformFootHeight) {
                    float height;
                    bool isOnPlatform = downPlatformFootHeight->getHeight(math::Vec2f(newCenter.x(), newCenter.y()), height);
                    newCenter[1] = height + size.y() * 0.5f;
                    float climb = newCenter.y() - oldCenter.y();
                    if (climb > climbCapacity) {
                        newCenter = oldCenter;
                    }               
                } else if (upPlatformHeadHeight != nullptr && downPlatformFootHeight != nullptr &&
                    upPlatformHeadHeight != downPlatformFootHeight) {
                    float height;
                    bool isOnPlatform = upPlatformHeadHeight->getHeight(math::Vec2f(newCenter.x(), newCenter.y()), height);
                    newCenter[1] = height + size.y() * 0.5f;
                    float climb = newCenter.y() - oldCenter.y();
                    if (climb > climbCapacity) {
                        newCenter = oldCenter;
                    }                
                } else if (upPlatformHeadHeight != nullptr && upPlatformFootHeight != nullptr
                           && upPlatformHeadHeight == upPlatformFootHeight) {
                    newCenter = newCenter - velocity;
                    float height;
                    bool isOnPlateform = upPlatformFootHeight->getHeight(math::Vec2f(newCenter.x(), newCenter.y()), height);
                    if (newCenter[1] - size.y() < height) {
                        newCenter[1] = height + size.y();
                    }                
                } else if (upPlatformHeadHeight != nullptr && upPlatformFootHeight != nullptr
                           && upPlatformHeadHeight != upPlatformFootHeight) {
                    float height;
                    bool isOnPlatform = upPlatformHeadHeight->getHeight(math::Vec2f(newCenter.x(), newCenter.y()), height);
                    newCenter[1] = height + size.y() * 0.5f;
                    float climb = newCenter.y() - oldCenter.y();
                    if (climb > climbCapacity) {
                        newCenter = oldCenter;
                    }
                }
                return newCenter;
            } else if (velocity.y() < 0) {
                if (upPlatformHeadHeight != nullptr && upPlatformFootHeight != nullptr
                    && upPlatformHeadHeight != upPlatformFootHeight) {
                    newCenter = newCenter - velocity;
                    float height;
                    bool isOnPlateform = upPlatformFootHeight->getHeight(math::Vec2f(newCenter.x(), newCenter.y()), height);
                    float climb = newCenter.y() - oldCenter.y();
                    if (climb > climbCapacity) {
                        newCenter = oldCenter;
                    }
                } else if (upPlatformHeadHeight != nullptr && upPlatformFootHeight != nullptr
                           && upPlatformHeadHeight == upPlatformFootHeight) {
                    newCenter = newCenter - velocity;
                }
                return newCenter;
            } else {
                if (upPlatformHeadHeight != nullptr && downPlatformFootHeight != nullptr &&
                    upPlatformHeadHeight == downPlatformFootHeight) {
                    float height;
                    bool isOnPlatform = downPlatformFootHeight->getHeight(math::Vec2f(newCenter.x(), newCenter.y()), height);
                    newCenter[1] = height + size.y() * 0.5f;
                    float climb = newCenter.y() - oldCenter.y();
                    if (climb > climbCapacity) {
                        newCenter = oldCenter;
                    }
                } else if (upPlatformHeadHeight != nullptr && upPlatformFootHeight != nullptr
                           && upPlatformHeadHeight == upPlatformFootHeight) {
                    float height;
                    bool isOnPlateform = upPlatformFootHeight->getHeight(math::Vec2f(newCenter.x(), newCenter.y()), height);
                    if (newCenter.y() - size.y() < height) {
                        newCenter[1] = height + size.y();
                    }
                }
                return newCenter;
            }
        }*/
    }
}
