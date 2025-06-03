#include "../../../include/odfaeg/Graphics/map.h"
#include "../../../include/odfaeg/Graphics/rectangleShape.h"
#include "../../../include/odfaeg/Physics/boundingEllipsoid.h"
#include <iostream>
#include <climits>
#include "../../../include/odfaeg/Core/singleton.h"
#include "../../../include/odfaeg/Graphics/tGround.h"
#include "../../../include/odfaeg/Graphics/boneAnimation.hpp"
//#include "../../../include/odfaeg/Graphics/application.h"
namespace odfaeg {
    namespace graphic {
    using namespace std;
    using namespace g3d;

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
        int Scene::getCellDepth() {
            return cellDepth;
        }
        void Scene::generate_labyrinthe (std::vector<Tile*> tGround, std::vector<g2d::Wall*> walls, math::Vec2f tileSize, physic::BoundingBox &rect, EntityFactory& factory) {
            int startX = rect.getPosition().x / tileSize.x * tileSize.x;
            int startY = rect.getPosition().y / tileSize.y * tileSize.y;
            int endX = (rect.getPosition().x + rect.getWidth()) / tileSize.x * tileSize.x;
            int endY = (rect.getPosition().y + rect.getHeight()) / tileSize.y * tileSize.y;
            BigTile *bt = factory.make_entity<BigTile>(math::Vec3f(startX, startY, startY + endY * 0.5f), factory);
            bt->setSize(rect.getSize());
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
                    } else {
                        Entity* tile;
                        if (tGround.size() > 0)  {
                            int i = math::Math::random(tGround.size());
                            tile = tGround[i]->clone();
                            tile->setPosition(math::Vec3f(pos.x, pos.y, pos.y + tile->getSize().y * 0.5f));
                            ////std::cout<<"add tile : "<<tile->getPosition()<<std::endl;
                        } else {
                            tile = factory.make_entity<Tile>(nullptr, math::Vec3f(pos.x, pos.y, pos.y + tileSize.y * 0.5f), math::Vec3f(tileSize.x, tileSize.y, 0), sf::IntRect(0, 0, tileSize.x, tileSize.y), factory);
                        }
                        bt->addTile(tile);
                    }
                }
            }
            addEntity(bt);
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
                std::vector<Entity*> wall = cell->getEntitiesInside("E_WALL");
                for (unsigned int i = 0; i < wall.size(); i++) {
                    deleteEntity(wall[i]);
                }
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
         void Scene::generate_3d_labyrinthe (std::vector<Tile*> tGround, std::vector<g3d::Wall*> walls, math::Vec2f tileSize, physic::BoundingBox &rect, EntityFactory& factory) {
            int startX = rect.getPosition().x / tileSize.x * tileSize.x;
            int startY = rect.getPosition().z / tileSize.y * tileSize.y;
            int endX = (rect.getPosition().x + rect.getWidth()) / tileSize.x * tileSize.x;
            int endY = (rect.getPosition().z + rect.getHeight()) / tileSize.y * tileSize.y;
            BigTile *bt = factory.make_entity<BigTile>(math::Vec3f(startX, rect.getPosition().y, startY),factory, tileSize,rect.getWidth() / tileSize.x);
            bt->setSize(rect.getSize());
            unsigned int i, j;
            for (int y = startY, j = 0; y < endY; y+= tileSize.y, j++) {
                for (int x = startX, i = 0; x < endX; x+= tileSize.x, i++) {
                    math::Vec3f projPos = gridMap->getBaseChangementMatrix().changeOfBase(math::Vec3f (x - startX, y - startY, 0));
                    math::Vec2f pos (projPos.x + startX, projPos.y + startY);
                    if (x == startX && y == startY) {
                        Entity *w = walls[Wall::TOP_LEFT]->clone();
                        w->setPosition(math::Vec3f(pos.x, rect.getPosition().z,pos.y));
                        w->setSize(math::Vec3f(w->getSize().x, rect.getSize().y + w->getSize().y, w->getSize().z));
                        addEntity(w);
                        gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, rect.getPosition().y, w->getPosition().y))->setPassable(false);
                    } else if (x == endX && y == startY) {
                        Entity *w = walls[Wall::TOP_RIGHT]->clone();
                        w->setPosition(math::Vec3f(pos.x, rect.getPosition().z,pos.y));
                        w->setSize(math::Vec3f(w->getSize().x, rect.getSize().y + w->getSize().y, w->getSize().z));
                        addEntity(w);
                        gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, rect.getPosition().y, w->getPosition().y))->setPassable(false);
                    } else if (y == endY && x == endX) {
                        Entity *w = walls[Wall::BOTTOM_RIGHT]->clone();
                        w->setPosition(math::Vec3f(pos.x, rect.getPosition().z,pos.y));
                        w->setSize(math::Vec3f(w->getSize().x, rect.getSize().y + w->getSize().y, w->getSize().z));
                        addEntity(w);
                        gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, rect.getPosition().y, w->getPosition().y))->setPassable(false);
                    } else if (x == startX && y == endY) {
                        Entity *w = walls[Wall::BOTTOM_LEFT]->clone();
                        w->setPosition(math::Vec3f(pos.x, rect.getPosition().z,pos.y));
                        w->setSize(math::Vec3f(w->getSize().x, rect.getSize().y + w->getSize().y, w->getSize().z));
                        addEntity(w);
                        gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, rect.getPosition().y, w->getPosition().y))->setPassable(false);
                    } else if (y == startY && j % 2 != 0) {
                        Entity *w = walls[Wall::TOP_BOTTOM]->clone();
                        w->setPosition(math::Vec3f(pos.x, rect.getPosition().z,pos.y));
                        w->setSize(math::Vec3f(w->getSize().x, rect.getSize().y + w->getSize().y, w->getSize().z));
                        addEntity(w);
                        gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, rect.getPosition().y, w->getPosition().y))->setPassable(false);
                    } else if (y == startY && j % 2 == 0) {
                        Entity *w = walls[Wall::T_TOP]->clone();
                        w->setPosition(math::Vec3f(pos.x, rect.getPosition().z,pos.y));
                        w->setSize(math::Vec3f(w->getSize().x, rect.getSize().y + w->getSize().y, w->getSize().z));
                        addEntity(w);
                        gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, rect.getPosition().y, w->getPosition().y))->setPassable(false);
                    } else if (x == endX && j % 2 != 0) {
                        Entity *w = walls[Wall::RIGHT_LEFT]->clone();
                        w->setPosition(math::Vec3f(pos.x, rect.getPosition().z,pos.y));
                        w->setSize(math::Vec3f(w->getSize().x, rect.getSize().y + w->getSize().y, w->getSize().z));
                        addEntity(w);
                        gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, rect.getPosition().y, w->getPosition().y))->setPassable(false);
                    } else if (x == endX && j % 2 == 0) {
                        Entity *w = walls[Wall::T_RIGHT]->clone();
                        w->setPosition(math::Vec3f(pos.x, rect.getPosition().z,pos.y));
                        w->setSize(math::Vec3f(w->getSize().x, rect.getSize().y + w->getSize().y, w->getSize().z));
                        addEntity(w);
                        gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, rect.getPosition().y, w->getPosition().y))->setPassable(false);
                    } else if(y == endY && i % 2 != 0) {
                        Entity *w = walls[Wall::TOP_BOTTOM]->clone();
                        w->setPosition(math::Vec3f(pos.x, rect.getPosition().z,pos.y));
                        w->setSize(math::Vec3f(w->getSize().x, rect.getSize().y + w->getSize().y, w->getSize().z));
                        addEntity(w);
                        gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, rect.getPosition().y, w->getPosition().y))->setPassable(false);
                    } else if (y == endY && i % 2 == 0) {
                        Entity *w = walls[Wall::T_BOTTOM]->clone();
                        w->setPosition(math::Vec3f(pos.x, rect.getPosition().z,pos.y));
                        w->setSize(math::Vec3f(w->getSize().x, rect.getSize().y + w->getSize().y, w->getSize().z));
                        addEntity(w);
                        gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, rect.getPosition().y, w->getPosition().y))->setPassable(false);
                    } else if (x == startX && j % 2 != 0) {
                        Entity *w = walls[Wall::RIGHT_LEFT]->clone();
                        w->setPosition(math::Vec3f(pos.x, rect.getPosition().z,pos.y));
                        w->setSize(math::Vec3f(w->getSize().x, rect.getSize().y + w->getSize().y, w->getSize().z));
                        addEntity(w);
                        gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, rect.getPosition().y, w->getPosition().y))->setPassable(false);
                    } else if (x == startX && j % 2 == 0) {
                        Entity *w = walls[Wall::T_LEFT]->clone();
                        w->setPosition(math::Vec3f(pos.x, rect.getPosition().z,pos.y));
                        w->setSize(math::Vec3f(w->getSize().x, rect.getSize().y + w->getSize().y, w->getSize().z));
                        addEntity(w);
                        gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, rect.getPosition().y, w->getPosition().y))->setPassable(false);
                    } else if (j % 2 != 0 && i % 2 == 0) {
                        Entity *w = walls[Wall::RIGHT_LEFT]->clone();
                        w->setPosition(math::Vec3f(pos.x, rect.getPosition().z,pos.y));
                        w->setSize(math::Vec3f(w->getSize().x, rect.getSize().y + w->getSize().y, w->getSize().z));
                        addEntity(w);
                        gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, rect.getPosition().y, w->getPosition().y))->setPassable(false);
                    } else if (j % 2 == 0 && i % 2 != 0) {
                        Entity *w = walls[Wall::TOP_BOTTOM]->clone();
                        w->setPosition(math::Vec3f(pos.x, rect.getPosition().z,pos.y));
                        w->setSize(math::Vec3f(w->getSize().x, rect.getSize().y + w->getSize().y, w->getSize().z));
                        addEntity(w);
                        gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, rect.getPosition().y, w->getPosition().y))->setPassable(false);
                    } else if (j % 2 == 0 && i % 2 == 0) {
                        Entity *w = walls[Wall::X]->clone();
                        w->setPosition(math::Vec3f(pos.x, rect.getPosition().z,pos.y));
                        w->setSize(math::Vec3f(w->getSize().x, rect.getSize().y + w->getSize().y, w->getSize().z));
                        addEntity(w);
                        gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, rect.getPosition().y, w->getPosition().y))->setPassable(false);
                    } else {
                        Entity* tile;
                        if (tGround.size() > 0)  {
                            int i = math::Math::random(tGround.size());
                            tile = tGround[i]->clone();
                            tile->setPosition(math::Vec3f(pos.x, 0, pos.y));
                            ////std::cout<<"add tile : "<<tile->getPosition()<<std::endl;
                        } else {
                            tile = factory.make_entity<Tile>(nullptr, math::Vec3f(pos.x, 0, pos.y), math::Vec3f(tileSize.x, 0, tileSize.y), sf::IntRect(0, 0, tileSize.x, tileSize.y), factory);
                        }
                        float heights[4];
                        for (unsigned int j = 0; j < sizeof(heights) / sizeof(float); j++) {
                            heights[j] = math::Math::random(rect.getPosition().y, rect.getPosition().y + rect.getHeight());
                        }
                        bt->addTile(tile, math::Vec2f(pos.x, pos.y), heights);
                    }
                }
            }
            addEntity(bt);
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
                    CellMap* cell = gridMap->getGridCellAt(math::Vec3f(pos.x, rect.getPosition().y, pos.y));
                    if (cell != nullptr) {
                        dirs.push_back(dir);
                    }
                }
                math::Vec2f dir = dirs[math::Math::random(0, dirs.size())];
                int x = cx + dir.x * tileSize.x;
                int y = cy + dir.y * tileSize.y;
                math::Vec3f projPos = gridMap->getBaseChangementMatrix().changeOfBase(math::Vec3f (x - startX, y - startY, 0));
                math::Vec2f pos (projPos.x + startX, projPos.y + startY);
                CellMap* cell = gridMap->getGridCellAt(math::Vec3f(pos.x, rect.getPosition().y, pos.y));
                std::vector<Entity*> wall = cell->getEntitiesInside("E_WALL");
                for (unsigned int i = 0; i < wall.size(); i++) {
                    deleteEntity(wall[i]);
                }
                cell->setPassable(true);
                cx += dir.x * tileSize.x * 2;
                cy += dir.y * tileSize.y * 2;
                x = cx, y = cy;
                projPos = gridMap->getBaseChangementMatrix().changeOfBase(math::Vec3f (x - startX, y - startY, 0));
                pos = math::Vec2f (projPos.x + startX, projPos.y + startY);
                cell = gridMap->getGridCellAt(math::Vec3f(pos.x, rect.getPosition().y, pos.y));
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
        void Scene::generate_map(std::vector<Tile*> tGround, std::vector<g2d::Wall*> walls, math::Vec2f tileSize, physic::BoundingBox &rect, EntityFactory& factory) {
            int startX = rect.getPosition().x / tileSize.x * tileSize.x;
            int startY = rect.getPosition().y / tileSize.y * tileSize.y;
            int endX = (rect.getPosition().x + rect.getWidth()) / tileSize.x * tileSize.x;
            int endY = (rect.getPosition().y + rect.getHeight()) / tileSize.y * tileSize.y;
            BigTile *bt = factory.make_entity<BigTile>(math::Vec3f(startX, startY, startY + (endY - startY) * 0.5f), factory);
            bt->setSize(rect.getSize());
            //bt->setCenter(math::Vec3f(rect.getCenter().x, rect.getCenter().y, rect.getPosition().z));
            //Positions de d\E9part et d'arriv\E9es en fonction de la taille, de la position et de la taille des cellules de la map.
            for (int y = startY; y < endY;  y+=tileSize.y) {
                for (int x = startX; x < endX; x+=tileSize.x) {
                    //std::cout<<"start x y : "<<startX<<","<<startY<<std::endl;
                    //std::cout<<"end x y : "<<endX-tileSize.x<<","<<endY-tileSize.y<<std::endl;
                    //std::cout<<"x y : "<<x<<","<<y<<std::endl;
                    //On projete les positions en fonction de la projection du jeux.
                    math::Vec3f projPos = gridMap->getBaseChangementMatrix().changeOfBase(math::Vec3f (x - startX, y - startY, 0));
                    math::Vec2f pos (projPos.x + startX, projPos.y + startY);
                    ////std::cout<<"pos : "<<pos<<std::endl;
                    //Mur du coin en haut \E0 gauche.
                    if (x == startX && y == startY && walls.size() >= 11) {
                        if (walls[Wall::TOP_LEFT] != nullptr) {
                            //std::cout<<"top left"<<std::endl;
                            Entity *w = walls[Wall::TOP_LEFT]->clone();
                            w->setPosition(math::Vec3f(pos.x, pos.y, pos.y + walls[Wall::TOP_LEFT]->getSize().y * 0.5f));
                            ////std::cout<<"position top right : "<<w->getPosition()<<std::endl;
                            addEntity(w);
                            gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, w->getPosition().z))->setPassable(false);
                        }

                        //Mur du coin en haut \E0 droite.
                    } else if (x == endX - tileSize.x && y == startY && walls.size() >= 11) {
                        if (walls[Wall::TOP_RIGHT] != nullptr) {
                            //std::cout<<"top right"<<std::endl;
                            Entity *w = walls[Wall::TOP_RIGHT]->clone();
                            w->setPosition(math::Vec3f(pos.x, pos.y, pos.y + walls[Wall::TOP_RIGHT]->getSize().y * 0.5f));
                            ////std::cout<<"position top right : "<<w->getPosition()<<std::endl;
                            addEntity(w);
                            gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, w->getPosition().z))->setPassable(false);
                        }
                        //Mur du coin en bas \E0 droite.
                    } else if (x == endX - tileSize.x && y == endY - tileSize.y && walls.size() >= 11) {
                        if (walls[Wall::BOTTOM_RIGHT] != nullptr) {
                            //std::cout<<"bottom right"<<std::endl;
                            Entity *w = walls[Wall::BOTTOM_RIGHT]->clone();
                            w->setPosition(math::Vec3f(pos.x, pos.y, pos.y + walls[Wall::BOTTOM_RIGHT]->getSize().y * 0.5f));
                            addEntity(w);
                            gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, w->getPosition().z))->setPassable(false);
                        }
                    } else if (x == startX && y == endY - tileSize.y && walls.size() >= 11) {
                        if (walls[Wall::BOTTOM_LEFT] != nullptr) {
                            //std::cout<<"bottom left"<<std::endl;
                            Entity *w = walls[Wall::BOTTOM_LEFT]->clone();
                            w->setPosition(math::Vec3f(pos.x, pos.y, pos.y + walls[Wall::BOTTOM_LEFT]->getSize().y * 0.5f));
                            ////std::cout<<"position bottom left : "<<w->getPosition()<<std::endl;
                            addEntity(w);
                            gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, w->getPosition().z))->setPassable(false);
                        }
                    } else if ((y == startY || y == endY - tileSize.y) && walls.size() >= 11) {
                        if (walls[Wall::TOP_BOTTOM] != nullptr) {
                            //std::cout<<"top bottom"<<std::endl;
                            Entity *w = walls[Wall::TOP_BOTTOM]->clone();
                            w->setPosition(math::Vec3f(pos.x, pos.y, pos.y + walls[Wall::TOP_BOTTOM]->getSize().y * 0.5f));
                            addEntity(w);
                            if (getBaseChangementMatrix().isIso2DMatrix() && y == endY - tileSize.y) {
                                int i = math::Math::random(tGround.size());
                                Entity *tile = tGround[i]->clone();
                                tile->setPosition(math::Vec3f(pos.x, pos.y, pos.y + tile->getSize().y * 0.5f));
                                bt->addTile(tile);
                            }
                            gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, w->getPosition().z))->setPassable(false);
                        }
                    } else if ((x == startX || x == endX - tileSize.x) && walls.size() >= 11) {
                        if (walls[Wall::RIGHT_LEFT] != nullptr) {
                            //std::cout<<"right left"<<std::endl;
                            Entity *w = walls[Wall::RIGHT_LEFT]->clone();

                            w->setPosition(math::Vec3f(pos.x, pos.y, pos.y + walls[Wall::RIGHT_LEFT]->getSize().y * 0.5f));
                            addEntity(w);
                            if (getBaseChangementMatrix().isIso2DMatrix() && x == endX - tileSize.x) {
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
                            ////std::cout<<"add tile : "<<tile->getPosition()<<std::endl;
                        } else {
                            tile = factory.make_entity<Tile>(nullptr, math::Vec3f(pos.x, pos.y, pos.y + tileSize.y * 0.5f), math::Vec3f(tileSize.x, tileSize.y, 0), sf::IntRect(0, 0, tileSize.x, tileSize.y), factory);
                        }
                        bt->addTile(tile);
                    }
                }
            }
            addEntity(bt);
        }
        void Scene::generate_3d_map(std::vector<Tile*> tGround, std::vector<g3d::Wall*> walls, math::Vec2f tileSize, physic::BoundingBox &rect, EntityFactory& factory) {
            int startX = rect.getPosition().x / tileSize.x * tileSize.x;
            int startY = rect.getPosition().z / tileSize.y * tileSize.y;
            int endX = (rect.getPosition().x + rect.getWidth()) / tileSize.x * tileSize.x;
            int endY = (rect.getPosition().z + rect.getDepth()) / tileSize.y * tileSize.y;
            BigTile *bt = factory.make_entity<BigTile>(math::Vec3f(startX, rect.getPosition().y, startY),factory, tileSize,rect.getWidth() / tileSize.x);
            bt->setSize(rect.getSize());
            //bt->setCenter(math::Vec3f(rect.getCenter().x, rect.getCenter().y, rect.getPosition().z));
            //Positions de d\E9part et d'arriv\E9es en fonction de la taille, de la position et de la taille des cellules de la map.
            for (int y = startY; y < endY;  y+=tileSize.y) {
                for (int x = startX; x < endX; x+=tileSize.x) {
                    //std::cout<<"start x y : "<<startX<<","<<startY<<std::endl;
                    //std::cout<<"end x y : "<<endX-tileSize.x<<","<<endY-tileSize.y<<std::endl;
                    //std::cout<<"x y : "<<x<<","<<y<<std::endl;
                    //On projete les positions en fonction de la projection du jeux.
                    math::Vec3f projPos = gridMap->getBaseChangementMatrix().changeOfBase(math::Vec3f (x - startX, y - startY, 0));
                    math::Vec2f pos (projPos.x + startX, projPos.y + startY);
                    ////std::cout<<"pos : "<<pos<<std::endl;
                    //Mur du coin en haut \E0 gauche.
                    if (x == startX && y == startY && walls.size() >= 11) {
                        if (walls[Wall::TOP_LEFT] != nullptr) {
                            //std::cout<<"top left"<<std::endl;
                            Entity *w = walls[Wall::TOP_LEFT]->clone();
                            w->setPosition(math::Vec3f(pos.x, rect.getPosition().y, pos.y));
                            w->setSize(math::Vec3f(w->getSize().x, rect.getSize().z + w->getSize().z, w->getSize().z));
                            ////std::cout<<"position top right : "<<w->getPosition()<<std::endl;
                            addEntity(w);
                            gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, w->getPosition().z))->setPassable(false);
                        }

                        //Mur du coin en haut \E0 droite.
                    } else if (x == endX - tileSize.x && y == startY && walls.size() >= 11) {
                        if (walls[Wall::TOP_RIGHT] != nullptr) {
                            //std::cout<<"top right"<<std::endl;
                            Entity *w = walls[Wall::TOP_RIGHT]->clone();
                            w->setPosition(math::Vec3f(pos.x, rect.getPosition().y, pos.y));
                            w->setSize(math::Vec3f(w->getSize().x, rect.getSize().z + w->getSize().z, w->getSize().z));
                            ////std::cout<<"position top right : "<<w->getPosition()<<std::endl;
                            addEntity(w);
                            gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, w->getPosition().z))->setPassable(false);
                        }
                        //Mur du coin en bas \E0 droite.
                    } else if (x == endX - tileSize.x && y == endY - tileSize.y && walls.size() >= 11) {
                        if (walls[Wall::BOTTOM_RIGHT] != nullptr) {
                            //std::cout<<"bottom right"<<std::endl;
                            Entity *w = walls[Wall::BOTTOM_RIGHT]->clone();
                            w->setPosition(math::Vec3f(pos.x, rect.getPosition().y, pos.y));
                            w->setSize(math::Vec3f(w->getSize().x, rect.getSize().z + w->getSize().z, w->getSize().z));
                            addEntity(w);
                            gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, w->getPosition().z))->setPassable(false);
                        }
                    } else if (x == startX && y == endY - tileSize.y && walls.size() >= 11) {
                        if (walls[Wall::BOTTOM_LEFT] != nullptr) {
                            //std::cout<<"bottom left"<<std::endl;
                            Entity *w = walls[Wall::BOTTOM_LEFT]->clone();
                            w->setPosition(math::Vec3f(pos.x, rect.getPosition().y, pos.y));
                            w->setSize(math::Vec3f(w->getSize().x, rect.getSize().z + w->getSize().z, w->getSize().z));
                            ////std::cout<<"position bottom left : "<<w->getPosition()<<std::endl;
                            addEntity(w);
                            gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, w->getPosition().z))->setPassable(false);
                        }
                    } else if ((y == startY || y == endY - tileSize.y) && walls.size() >= 11) {
                        if (walls[Wall::TOP_BOTTOM] != nullptr) {
                            //std::cout<<"top bottom"<<std::endl;
                            Entity *w = walls[Wall::TOP_BOTTOM]->clone();
                            w->setPosition(math::Vec3f(pos.x, rect.getPosition().y, pos.y));
                            w->setSize(math::Vec3f(w->getSize().x, rect.getSize().z + w->getSize().z, w->getSize().z));
                            addEntity(w);
                            /*if (y == endY - tileSize.y) {
                                int i = math::Math::random(tGround.size());
                                Entity *tile = tGround[i]->clone();
                                tile->setPosition(math::Vec3f(pos.x, 0, pos.y));
                                bt->addTile(tile);
                            }*/
                            gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, w->getPosition().z))->setPassable(false);
                        }
                    } else if ((x == startX || x == endX - tileSize.x) && walls.size() >= 11) {
                        if (walls[Wall::RIGHT_LEFT] != nullptr) {
                            //std::cout<<"right left"<<std::endl;
                            Entity *w = walls[Wall::RIGHT_LEFT]->clone();
                            w->setPosition(math::Vec3f(pos.x, rect.getPosition().y, pos.y));
                            w->setSize(math::Vec3f(w->getSize().x, rect.getSize().z + w->getSize().z, w->getSize().z));
                            addEntity(w);
                            /*if (x == endX - tileSize.x) {
                                int i = math::Math::random(tGround.size());
                                Entity *tile = tGround[i]->clone();
                                tile->setPosition(math::Vec3f(pos.x, 0, pos.y));
                                bt->addTile(tile);
                            }*/
                            gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, w->getPosition().z))->setPassable(false);
                        }
                    } else {
                        Entity* tile;
                        if (tGround.size() > 0)  {
                            int i = math::Math::random(tGround.size());
                            tile = tGround[i]->clone();
                            tile->setPosition(math::Vec3f(pos.x, 0, pos.y));
                        } else {
                            tile = factory.make_entity<Tile>(nullptr, math::Vec3f(pos.x, 0, pos.y), math::Vec3f(tileSize.x, 0, tileSize.y), sf::IntRect(0, 0, tileSize.x, tileSize.y), factory);
                        }
                        float heights[4];
                        for (unsigned int j = 0; j < sizeof(heights) / sizeof(float); j++) {
                            heights[j] = math::Math::random(rect.getPosition().y, rect.getPosition().y + rect.getHeight());
                        }
                        bt->addTile(tile, math::Vec2f(pos.x, pos.y), heights);
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
                    ////std::cout<<"bone index : "<<entity->getCurrentFrame()->getBoneIndex()<<std::endl;
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
            //std::cout<<"add entity : "<<entity->getType()<<std::endl;
            return gridMap->addEntity(entity);
        }
        bool Scene::removeEntity (Entity *entity) {
            bool removed = true;
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
                //std::cout<<"remove entity : "<<entity->getType()<<std::endl;
                if(!gridMap->removeEntity(entity)) {
                   removed = false;
                }
                std::vector<Face> faces = entity->getFaces();
                for (unsigned int j = 0; j < faces.size(); j++) {
                    decreaseComptImg(faces[j].getMaterial().getTexture());
                }
            }
            return removed;
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
                ////std::cout<<"remove entity : "<<entity->getType()<<std::endl;
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
        void Scene::rotateEntity(Entity *entity, float angle, math::Vec3f axis) {
            removeEntity(entity);
            entity->setRotation(angle, axis);
            addEntity(entity);
        }
        void Scene::scaleEntity(Entity *entity, float sx, float sy, float sz) {
            removeEntity(entity);
            entity->setScale(math::Vec3f(sx, sy, sz));
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
                                            visibleEntities[entity->getRootTypeInt()][entity->getId()] = entity;
                                       }

                                    }
                                }
                            }
                        }
                    }
                }
                /*for (unsigned int i = 0; i < visibleEntities.size(); i++) {
                    if (i == 0) {
                        for (unsigned int j = 0; j < visibleEntities[0].size(); j++) {
                            if (visibleEntities[0][j] != nullptr && visibleEntities[0][j]->getRootEntity()->getType() == "E_WALL" && visibleEntities[0][j]->getRootEntity()->getWallType() == 2)
                                //std::cout<<"wall"<<std::endl;
                        }
                    }
                }*/

                if (c < frcm->getNbComponents() && frcm->getRenderComponent(c) != nullptr) {
                    ////std::cout<<"get entities on component : "<<c<<std::endl;
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
            /*std::vector<Entity*> allEntities = gridMap->getEntities();
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
            return nullptr;*/
            return gridMap->getEntity(name);
        }
        Entity* Scene::checkFrameEntity(Entity* frame, std::string name) {
            if (frame->getName() == name)
                return frame;
            for (unsigned int i = 0; i < frame->getChildren().size(); i++) {
                return checkFrameEntity(frame->getChildren()[i], name);
            }
            return nullptr;
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
                            if (entity->isAnimated()) {
                                addAnimationChildren(entity, entities, type);
                            } else {
                                entities.push_back(entity);
                            }
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
                            if (entity->isAnimated()) {
                                addAnimationChildren(entity, entities, type);
                            } else {
                                entities.push_back(entity);
                            }
                        }
                    }
                }
            }
            return entities;
        }
        void Scene::addAnimationChildren(Entity* entity, std::vector<Entity*>& entities, std::string type) {
            entities.push_back(entity);
            if (type.size() > 0 && type.at(0) == '*') {
                if (type.find("-") != string::npos)
                    type = type.substr(2, type.size() - 3);
                vector<string> excl = core::split(type, "-");
                for (unsigned int i = 0; i < entity->getChildren().size(); i++) {
                    bool exclude = false;
                    for (unsigned int j = 0; j < excl.size(); j++) {
                        if (entity->getChildren()[i]->getType() == excl[j])
                            exclude = true;
                    }
                    if (!exclude) {
                        addAnimationChildren(entity->getChildren()[i], entities, type);
                    }
               }
               return;
            }
            vector<string> types = core::split(type, "+");
            for (unsigned int i = 0; i < types.size(); i++) {
                for (unsigned int i = 0; i < entity->getChildren().size(); i++) {
                    if (entity->getChildren()[i]->getType() == types[i]) {
                         addAnimationChildren(entity->getChildren()[i], entities, type);
                    }
                }
            }
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

        vector<Entity*> Scene::getVisibleEntities (std::string expression, EntityFactory& factory) {

            std::vector<Entity*> entities;
            entities.resize(factory.getNbEntities(), nullptr);
            if (expression.size() > 0 && expression.at(0) == '*') {
                if (expression.find("-") != string::npos)
                    expression = expression.substr(2, expression.size() - 2);
                vector<string> excl = core::split(expression, "-");
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
                                    entities[visibleEntities[i][j]->getId()] = visibleEntities[i][j];
                                }
                            }
                        }
                    }
                }
                return entities;
            }
            vector<string> types = core::split(expression, "+");
            for (unsigned int t = 0; t < types.size(); t++) {

                unsigned int typeInt = factory.getIntOfType(types[t]);


                if (typeInt < visibleEntities.size()) {
                    vector<Entity*> visibleEntitiesType = visibleEntities[typeInt];
                    for (unsigned int i = 0; i < visibleEntitiesType.size(); i++) {
                        bool found = false;

                        if (visibleEntitiesType[i] != nullptr && visibleEntitiesType[i]->getRootType() ==  types[t]) {

                            found = true;
                        }

                        if (visibleEntitiesType[i] != nullptr && found) {
                            Entity* ba = visibleEntitiesType[i]->getRootEntity();
                            if (ba->getBoneAnimationIndex() == visibleEntitiesType[i]->getBoneIndex()) {
                                /*if (visibleEntitiesType[i]->getType() == "E_ANIMATION")
                                    //std::cout<<"add animation"<<visibleEntitiesType[i]->getCurrentFrame()->getType()<<std::endl;*/

                                entities[visibleEntitiesType[i]->getId()] = visibleEntitiesType[i];
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
             //std::cout<<"entities in rect :"<<allEntitiesInRect.size()<<std::endl;

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
        bool Scene::collide (Entity *entity, math::Vec3f position, physic::CollisionResultSet& cinfos) {
             return gridMap->collideWithEntity(entity, position, cinfos);;
        }
        bool Scene::collide (Entity *entity) {
             return gridMap->collideWithEntity(entity);
        }
        bool Scene::collide (Entity* entity, math::Ray ray, physic::CollisionResultSet& cinfos) {
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
        math::Vec3f Scene::physicallyBasedComputePos (Entity* entity, math::Vec3f oldCenter, math::Vec3f size, math::Vec3f velocity, float climbCapacity) {
            float newFootHeight, newHeadHeight;
            //La nouvelle position du joueur suivant les tests de la physique du jeux.
            math::Vec3f newCenter = entity->getCenter();
            newFootHeight = newCenter.y - size.y * 0.5f;
            newHeadHeight = newCenter.y + size.y * 0.5f;
            //On cherche d'abord les collisions par rapport aux plateformes.
            //On récupère toutes les plateformes.
            std::vector<Entity*> platforms = getRootEntities("E_BIGTILE");
            /*Si le joueur descend, il faut rechercher la plateforme sur laquelle le joueur se trouve.
            Si le joueur n'est pas en dessous de la plateforme on le remet sur la plateforme et ensuite en regarde
            si le joueur peut escalader la pente ou monter sur la nouvelle plateforme.*/
            //Si il y a plusieurs plateformes il faut rechercher sur laquelle le joueur se trouve, c'est à dire la plus proche en dessous.
            float minDist;
            unsigned int index;
            for (unsigned int i = 0; i< platforms.size(); i++) {
                //Hauteur de la plateforme ou le joueur se trouvait.
                float height;
                bool isOnPlateform = platforms[i]->getHeight(math::Vec2f(newCenter.x, newCenter.y), height);
                //Distance entre le pied du joueur et la plateforme et la capacité à monter sur la plateforme.
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
                bool isOnPlatform = platforms[i]->getHeight(math::Vec2f(newCenter.x, newCenter.y), height);
                //Distance entre le pied du joueur et la plateforme et la capacité à monter sur la pente ou la plateforme.
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
                bool isOnPlatform = platforms[i]->getHeight(math::Vec2f(newCenter.x, newCenter.y), height);
                //Distance entre le pied du joueur et la plateforme et la capacité à monter sur la plateforme.
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
                bool isOnPlatform = platforms[i]->getHeight(math::Vec2f(newCenter.x, newCenter.y), height);
                //Distance entre le pied du joueur et la plateforme et la capacité à monter sur la pente ou la plateforme.
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
                bool isOnPlatform = platforms[i]->getHeight(math::Vec2f(newCenter.x, newCenter.y), height);
                //Distance entre le pied du joueur et la plateforme et la capacité à monter sur la plateforme.
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
                bool isOnPlatform = platforms[i]->getHeight(math::Vec2f(newCenter.x, newCenter.y), height);
                //Distance entre le pied du joueur et la plateforme et la capacité à monter sur la pente ou la plateforme.
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
                bool isOnPlatform = platforms[i]->getHeight(math::Vec2f(newCenter.x, newCenter.y), height);
                //Distance entre le pied du joueur et la plateforme et la capacité à monter sur la plateforme.
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
                bool isOnPlatform = platforms[i]->getHeight(math::Vec2f(newCenter.x, newCenter.y), height);
                //Distance entre le pied du joueur et la plateforme et la capacité à monter sur la pente ou la plateforme.
                float dist = height - newHeadHeight;
                //Si le joueur n'est pas en dessous de la plateforme et que la plateforme est la plus proche on initialise la plateforme et la distance minimum et la plateforme.
                if (dist < minDist && dist >= 0 && isOnPlatform) {
                    minDist = dist;
                    downPlatformHeadHeight = platforms[i];
                }
            }

            if (velocity.y > 0) {
                /*Si la plateforme en dessous de la tête et la plateforme au dessus des pieds est la même,
                il faut récupérer la hauteur du pied sur la plateforme au dessus du pied et tester si le
                personnage peut grimper la pente, si oui, on avance, sinon, on ne bouge pas.*/
                if (upPlatformHeadHeight != nullptr && downPlatformFootHeight != nullptr &&
                    upPlatformHeadHeight == downPlatformFootHeight) {
                    float height;
                    bool isOnPlatform = downPlatformFootHeight->getHeight(math::Vec2f(newCenter.x, newCenter.y), height);
                    newCenter.y = height + size.y * 0.5f;
                    float climb = newCenter.y - oldCenter.y;
                    if (climb > climbCapacity) {
                        newCenter = oldCenter;
                    }
                /*Si la plateform en dessous de la tête n'est pas la même que celle au dessus des pieds,
                  il faut tester si le joueur peut grimper sur la nouvelle plateforme, si oui, on avance, sinon on n'avance pas.*/
                } else if (upPlatformHeadHeight != nullptr && downPlatformFootHeight != nullptr &&
                    upPlatformHeadHeight != downPlatformFootHeight) {
                    float height;
                    bool isOnPlatform = upPlatformHeadHeight->getHeight(math::Vec2f(newCenter.x, newCenter.y), height);
                    newCenter.y = height + size.y * 0.5f;
                    float climb = newCenter.y - oldCenter.y;
                    if (climb > climbCapacity) {
                        newCenter = oldCenter;
                    }
                /*Si la plateforme est en dessous de la tête est des pieds, on tombe, si on est plus bas que la plateforme on revient
                sur la plateforme.*/
                } else if (upPlatformHeadHeight != nullptr && upPlatformFootHeight != nullptr
                           && upPlatformHeadHeight == upPlatformFootHeight) {
                    newCenter = newCenter - velocity;
                    float height;
                    bool isOnPlateform = upPlatformFootHeight->getHeight(math::Vec2f(newCenter.x, newCenter.y), height);
                    if (newCenter.y - size.y < height) {
                        newCenter.y = height + size.y;
                    }
                /*Si la plateforme en dessous de la tête et celle en dessous des pieds n'est pas la même, on vérfifie si on peut grimper,
                si oui on avance si pas en reste sur place.*/
                } else if (upPlatformHeadHeight != nullptr && upPlatformFootHeight != nullptr
                           && upPlatformHeadHeight != upPlatformFootHeight) {
                    float height;
                    bool isOnPlatform = upPlatformHeadHeight->getHeight(math::Vec2f(newCenter.x, newCenter.y), height);
                    newCenter.y = height + size.y * 0.5f;
                    float climb = newCenter.y - oldCenter.y;
                    if (climb > climbCapacity) {
                        newCenter = oldCenter;
                    }
                }
                return newCenter;
            } else if (velocity.y < 0) {
                if (upPlatformHeadHeight != nullptr && upPlatformFootHeight != nullptr
                    && upPlatformHeadHeight != upPlatformFootHeight) {
                    newCenter = newCenter - velocity;
                    float height;
                    bool isOnPlateform = upPlatformFootHeight->getHeight(math::Vec2f(newCenter.x, newCenter.y), height);
                    float climb = newCenter.y - oldCenter.y;
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
                    bool isOnPlatform = downPlatformFootHeight->getHeight(math::Vec2f(newCenter.x, newCenter.y), height);
                    newCenter.y = height + size.y * 0.5f;
                    float climb = newCenter.y - oldCenter.y;
                    if (climb > climbCapacity) {
                        newCenter = oldCenter;
                    }
                } else if (upPlatformHeadHeight != nullptr && upPlatformFootHeight != nullptr
                           && upPlatformHeadHeight == upPlatformFootHeight) {
                    float height;
                    bool isOnPlateform = upPlatformFootHeight->getHeight(math::Vec2f(newCenter.x, newCenter.y), height);
                    if (newCenter.y - size.y < height) {
                        newCenter.y = height + size.y;
                    }
                }
                return newCenter;
            }
        }
    }
}

