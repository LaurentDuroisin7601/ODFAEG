#include "../../../../include/odfaeg/Graphics/ECS/scene.hpp"
//#include "../../../../include/odfaeg/Graphics/ECS/application.hpp"
using namespace std;
namespace odfaeg {
    namespace graphic {
        namespace ecs {
            Scene::Scene(ComponentMapping& componentMapping) : SceneManager (), componentMapping(componentMapping) {
                frcm = nullptr;
                cellWidth = cellHeight = cellDepth = 0;
                gridMap = nullptr;
                name = "";
            }
            Scene::Scene (RenderComponentManager* frcm, ComponentMapping& componentMapping, std::string name, int cellWidth, int cellHeight, int cellDepth) : SceneManager(), frcm(frcm), componentMapping(componentMapping), cellWidth(cellWidth), cellHeight(cellHeight), cellDepth(cellDepth) {
                gridMap = new Grid(componentMapping, cellWidth, cellHeight, cellDepth);
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
            void Scene::generate_labyrinthe (std::vector<EntityId> tGround, std::vector<EntityId> walls, math::Vec2f tileSize, physic::BoundingBox &rect) {
                int startX = rect.getPosition().x() / tileSize.x() * tileSize.x();
                int startY = rect.getPosition().y() / tileSize.y() * tileSize.y();
                int endX = (rect.getPosition().x() + rect.getWidth()) / tileSize.x() * tileSize.x();
                int endY = (rect.getPosition().y() + rect.getHeight()) / tileSize.y() * tileSize.y();
                EntityId bt = ModelFactory::createBigTileModel(componentMapping, math::Vec3f(startX, startY, startY + endY * 0.5f), math::Vec3f(endX - startX, endY - startY, 0));
                addEntity(bt);
                unsigned int i, j;
                 for (int y = startY, j = 0; y < endY; y+= tileSize.y(), j++) {
                        for (int x = startX, i = 0; x < endX; x+= tileSize.x(), i++) {
                            math::Vec3f projPos = gridMap->getBaseChangementMatrix().changeOfBase(math::Vec3f (x - startX, y - startY, 0));
                            math::Vec2f pos (projPos.x() + startX, projPos.y() + startY);
                            if (x == startX && y == startY) {
                                EntityId wallId = walls[WallType::TOP_LEFT];
                                EntityId clonedWall = componentMapping.clone<TransformComponent, EntityInfoComponent>(wallId);
                                MoveSystem moveSystem;
                                auto transform = componentMapping.getComponent<TransformComponent>(clonedWall);
                                math::Vec3f position = math::Vec3f(pos.x(), pos.y(), pos.y() + transform->size.y() * 0.5f);
                                auto mvparams = std::make_tuple(position);
                                std::vector<EntityId> w(1);
                                w[0] = clonedWall;
                                componentMapping.apply<TransformComponent>(moveSystem, w, mvparams);
                                addEntity(w[0]);
                                gridMap->getGridCellAt(componentMapping.getComponent<TransformComponent>(w[0])->position)->setPassable(false);
                            } else if (x == endX && y == startY) {
                                EntityId wallId = walls[WallType::TOP_RIGHT];
                                EntityId clonedWall = componentMapping.clone<TransformComponent, EntityInfoComponent>(wallId);
                                MoveSystem moveSystem;
                                auto transform = componentMapping.getComponent<TransformComponent>(clonedWall);
                                math::Vec3f position = math::Vec3f(pos.x(), pos.y(), pos.y() + transform->size.y() * 0.5f);
                                auto mvparams = std::make_tuple(position);
                                std::vector<EntityId> w(1);
                                w[0] = clonedWall;
                                componentMapping.apply<TransformComponent>(moveSystem, w, mvparams);
                                addEntity(w[0]);
                                gridMap->getGridCellAt(componentMapping.getComponent<TransformComponent>(w[0])->position)->setPassable(false);
                            } else if (y == endY && x == endX) {
                                EntityId wallId = walls[WallType::BOTTOM_RIGHT];
                                EntityId clonedWall = componentMapping.clone<TransformComponent, EntityInfoComponent>(wallId);
                                MoveSystem moveSystem;
                                auto transform = componentMapping.getComponent<TransformComponent>(clonedWall);
                                math::Vec3f position = math::Vec3f(pos.x(), pos.y(), pos.y() + transform->size.y() * 0.5f);
                                auto mvparams = std::make_tuple(position);
                                std::vector<EntityId> w(1);
                                w[0] = clonedWall;
                                componentMapping.apply<TransformComponent>(moveSystem, w, mvparams);
                                addEntity(w[0]);
                                gridMap->getGridCellAt(componentMapping.getComponent<TransformComponent>(w[0])->position)->setPassable(false);
                            }   else if (x == startX && y == endY) {
                                EntityId wallId = walls[WallType::BOTTOM_LEFT];
                                EntityId clonedWall = componentMapping.clone<TransformComponent, EntityInfoComponent>(wallId);
                                MoveSystem moveSystem;
                                auto transform = componentMapping.getComponent<TransformComponent>(clonedWall);
                                math::Vec3f position = math::Vec3f(pos.x(), pos.y(), pos.y() + transform->size.y() * 0.5f);
                                auto mvparams = std::make_tuple(position);
                                std::vector<EntityId> w(1);
                                w[0] = clonedWall;
                                componentMapping.apply<TransformComponent>(moveSystem, w, mvparams);
                                addEntity(w[0]);
                                gridMap->getGridCellAt(componentMapping.getComponent<TransformComponent>(w[0])->position)->setPassable(false);
                            } else if (y == startY && j % 2 != 0) {
                                EntityId wallId = walls[WallType::TOP_BOTTOM];
                                EntityId clonedWall = componentMapping.clone<TransformComponent, EntityInfoComponent>(wallId);
                                MoveSystem moveSystem;
                                auto transform = componentMapping.getComponent<TransformComponent>(clonedWall);
                                math::Vec3f position = math::Vec3f(pos.x(), pos.y(), pos.y() + transform->size.y() * 0.5f);
                                auto mvparams = std::make_tuple(position);
                                std::vector<EntityId> w(1);
                                w[0] = clonedWall;
                                componentMapping.apply<TransformComponent>(moveSystem, w, mvparams);
                                addEntity(w[0]);
                                gridMap->getGridCellAt(componentMapping.getComponent<TransformComponent>(w[0])->position)->setPassable(false);
                            } else if (y == startY && j % 2 == 0) {
                                EntityId wallId = walls[WallType::T_TOP];
                                EntityId clonedWall = componentMapping.clone<TransformComponent, EntityInfoComponent>(wallId);
                                MoveSystem moveSystem;
                                auto transform = componentMapping.getComponent<TransformComponent>(clonedWall);
                                math::Vec3f position = math::Vec3f(pos.x(), pos.y(), pos.y() + transform->size.y() * 0.5f);
                                auto mvparams = std::make_tuple(position);
                                std::vector<EntityId> w(1);
                                w[0] = clonedWall;
                                componentMapping.apply<TransformComponent>(moveSystem, w, mvparams);
                                addEntity(w[0]);
                                gridMap->getGridCellAt(componentMapping.getComponent<TransformComponent>(w[0])->position)->setPassable(false);
                            } else if (x == endX && j % 2 != 0) {
                                EntityId wallId = walls[WallType::RIGHT_LEFT];
                                EntityId clonedWall = componentMapping.clone<TransformComponent, EntityInfoComponent>(wallId);
                                MoveSystem moveSystem;
                                auto transform = componentMapping.getComponent<TransformComponent>(clonedWall);
                                math::Vec3f position = math::Vec3f(pos.x(), pos.y(), pos.y() + transform->size.y() * 0.5f);
                                auto mvparams = std::make_tuple(position);
                                std::vector<EntityId> w(1);
                                w[0] = clonedWall;
                                componentMapping.apply<TransformComponent>(moveSystem, w, mvparams);
                                addEntity(w[0]);
                                gridMap->getGridCellAt(componentMapping.getComponent<TransformComponent>(w[0])->position)->setPassable(false);
                            } else if (x == endX && j % 2 == 0) {
                                EntityId wallId = walls[WallType::TOP_BOTTOM];
                                EntityId clonedWall = componentMapping.clone<TransformComponent, EntityInfoComponent>(wallId);
                                MoveSystem moveSystem;
                                auto transform = componentMapping.getComponent<TransformComponent>(clonedWall);
                                math::Vec3f position = math::Vec3f(pos.x(), pos.y(), pos.y() + transform->size.y() * 0.5f);
                                auto mvparams = std::make_tuple(position);
                                std::vector<EntityId> w(1);
                                w[0] = clonedWall;
                                componentMapping.apply<TransformComponent>(moveSystem, w, mvparams);
                                addEntity(w[0]);
                                gridMap->getGridCellAt(componentMapping.getComponent<TransformComponent>(w[0])->position)->setPassable(false);
                            } else if (y == endY && i % 2 == 0) {
                                EntityId wallId = walls[WallType::T_BOTTOM];
                                EntityId clonedWall = componentMapping.clone<TransformComponent, EntityInfoComponent>(wallId);
                                MoveSystem moveSystem;
                                auto transform = componentMapping.getComponent<TransformComponent>(clonedWall);
                                math::Vec3f position = math::Vec3f(pos.x(), pos.y(), pos.y() + transform->size.y() * 0.5f);
                                auto mvparams = std::make_tuple(position);
                                std::vector<EntityId> w(1);
                                w[0] = clonedWall;
                                componentMapping.apply<TransformComponent>(moveSystem, w, mvparams);
                                addEntity(w[0]);
                                gridMap->getGridCellAt(componentMapping.getComponent<TransformComponent>(w[0])->position)->setPassable(false);
                            } else if (x == startX && j % 2 != 0) {
                                EntityId wallId = walls[WallType::RIGHT_LEFT];
                                EntityId clonedWall = componentMapping.clone<TransformComponent, EntityInfoComponent>(wallId);
                                MoveSystem moveSystem;
                                auto transform = componentMapping.getComponent<TransformComponent>(clonedWall);
                                math::Vec3f position = math::Vec3f(pos.x(), pos.y(), pos.y() + transform->size.y() * 0.5f);
                                auto mvparams = std::make_tuple(position);
                                std::vector<EntityId> w(1);
                                w[0] = clonedWall;
                                componentMapping.apply<TransformComponent>(moveSystem, w, mvparams);
                                addEntity(w[0]);
                                gridMap->getGridCellAt(componentMapping.getComponent<TransformComponent>(w[0])->position)->setPassable(false);
                            } else if (x == startX && j % 2 == 0) {
                                EntityId wallId = walls[WallType::T_LEFT];
                                EntityId clonedWall = componentMapping.clone<TransformComponent, EntityInfoComponent>(wallId);
                                MoveSystem moveSystem;
                                auto transform = componentMapping.getComponent<TransformComponent>(clonedWall);
                                math::Vec3f position = math::Vec3f(pos.x(), pos.y(), pos.y() + transform->size.y() * 0.5f);
                                auto mvparams = std::make_tuple(position);
                                std::vector<EntityId> w(1);
                                w[0] = clonedWall;
                                componentMapping.apply<TransformComponent>(moveSystem, w, mvparams);
                                addEntity(w[0]);
                                gridMap->getGridCellAt(componentMapping.getComponent<TransformComponent>(w[0])->position)->setPassable(false);
                            } else if (j % 2 != 0 && i % 2 == 0) {
                                EntityId wallId = walls[WallType::RIGHT_LEFT];
                                EntityId clonedWall = componentMapping.clone<TransformComponent, EntityInfoComponent>(wallId);
                                MoveSystem moveSystem;
                                auto transform = componentMapping.getComponent<TransformComponent>(clonedWall);
                                math::Vec3f position = math::Vec3f(pos.x(), pos.y(), pos.y() + transform->size.y() * 0.5f);
                                auto mvparams = std::make_tuple(position);
                                std::vector<EntityId> w(1);
                                w[0] = clonedWall;
                                componentMapping.apply<TransformComponent>(moveSystem, w, mvparams);
                                addEntity(w[0]);
                                gridMap->getGridCellAt(componentMapping.getComponent<TransformComponent>(w[0])->position)->setPassable(false);
                            } else if (j % 2 == 0 && i % 2 != 0) {
                                EntityId wallId = walls[WallType::TOP_BOTTOM];
                                EntityId clonedWall = componentMapping.clone<TransformComponent, EntityInfoComponent>(wallId);
                                MoveSystem moveSystem;
                                auto transform = componentMapping.getComponent<TransformComponent>(clonedWall);
                                math::Vec3f position = math::Vec3f(pos.x(), pos.y(), pos.y() + transform->size.y() * 0.5f);
                                auto mvparams = std::make_tuple(position);
                                std::vector<EntityId> w(1);
                                w[0] = clonedWall;
                                componentMapping.apply<TransformComponent>(moveSystem, w, mvparams);
                                addEntity(w[0]);
                                gridMap->getGridCellAt(componentMapping.getComponent<TransformComponent>(w[0])->position)->setPassable(false);
                            } else if (j % 2 == 0 && i % 2 == 0) {
                                EntityId wallId = walls[WallType::X];
                                EntityId clonedWall = componentMapping.clone<TransformComponent, EntityInfoComponent>(wallId);
                                MoveSystem moveSystem;
                                auto transform = componentMapping.getComponent<TransformComponent>(clonedWall);
                                math::Vec3f position = math::Vec3f(pos.x(), pos.y(), pos.y() + transform->size.y() * 0.5f);
                                auto mvparams = std::make_tuple(position);
                                std::vector<EntityId> w(1);
                                w[0] = clonedWall;
                                componentMapping.apply<TransformComponent>(moveSystem, w, mvparams);
                                addEntity(w[0]);
                                gridMap->getGridCellAt(componentMapping.getComponent<TransformComponent>(w[0])->position)->setPassable(false);
                            }
                        }
                    }
                    std::vector<Cell*> visited;
                    std::vector<math::Vec2f> dirs;
                    int n = 0, cx = startX + tileSize.x(), cy = startY + tileSize.y();
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
                            x += dir.x() * tileSize.x() * 2;
                            y += dir.y() * tileSize.y() * 2;
                            math::Vec3f projPos = gridMap->getBaseChangementMatrix().changeOfBase(math::Vec3f (x - startX, y - startY, 0));
                            math::Vec2f pos (projPos.x() + startX, projPos.y() + startY);
                            Cell* cell = gridMap->getGridCellAt(math::Vec2f(pos.x(), pos.y()));
                            if (cell != nullptr) {
                                dirs.push_back(dir);
                            }
                        }
                        math::Vec2f dir = dirs[math::Math::random(0, dirs.size())];
                        int x = cx + dir.x() * tileSize.x();
                        int y = cy + dir.y() * tileSize.y();
                        math::Vec3f projPos = gridMap->getBaseChangementMatrix().changeOfBase(math::Vec3f (x - startX, y - startY, 0));
                        math::Vec2f pos (projPos.x() + startX, projPos.y() + startY);
                        Cell* cell = gridMap->getGridCellAt(math::Vec2f(pos.x(), pos.y()));
                        cell->removeEntity("E_WALL");
                        cell->setPassable(true);
                        cx += dir.x() * tileSize.x() * 2;
                        cy += dir.y() * tileSize.y() * 2;
                        x = cx, y = cy;
                        projPos = gridMap->getBaseChangementMatrix().changeOfBase(math::Vec3f (x - startX, y - startY, 0));
                        pos = math::Vec2f (projPos.x() + startX, projPos.y() + startY);
                        cell = gridMap->getGridCellAt(math::Vec2f(pos.x(), pos.y()));
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
            void Scene::generate_map(std::vector<EntityId> tGround, std::vector<EntityId> walls, math::Vec2f tileSize, physic::BoundingBox &rect) {
                    int startX = rect.getPosition().x() / tileSize.x() * tileSize.x();
                    int startY = rect.getPosition().y() / tileSize.y() * tileSize.y();
                    int endX = (rect.getPosition().x() + rect.getWidth()) / tileSize.x() * tileSize.x();
                    int endY = (rect.getPosition().y() + rect.getHeight()) / tileSize.y() * tileSize.y();
                    EntityId bt = ModelFactory::createBigTileModel(componentMapping, math::Vec3f(startX, startY, startY + (endY - startY) * 0.5f), math::Vec3f(endX - startX, endY - startY, 0));


                    for (int y = startY; y < endY;  y+=tileSize.y()) {
                        for (int x = startX; x < endX; x+=tileSize.x()) {
                            //On projete les positions en fonction de la projection du jeux.
                            math::Vec3f projPos = getBaseChangementMatrix().changeOfBase(math::Vec3f (x - startX, y - startY, 0));
                            math::Vec2f pos (projPos.x() + startX, projPos.y() + startY);
                            if (x == startX && y == startY && walls.size() >= 11) {
                                EntityId wall = walls[WallType::TOP_LEFT];
                                EntityId clonedWall;
                                clonedWall = componentMapping.clone<EntityInfoComponent, TransformComponent, WallTypeComponent, MeshComponent, ShadowInfoComponent>(wall);
                                /*std::vector<EntityId> children = componentMapping.getChildren(clonedWall);
                                for (unsigned int i = 0; i < children.size(); i++) {
                                    if (componentMapping.getComponent<MeshComponent>(children[i]) != nullptr)
                                        ////std::cout<<"mesh component"<<std::endl;
                                }*/
                                MoveSystem moveSystem;
                                auto transform = componentMapping.getComponent<TransformComponent>(clonedWall);
                                math::Vec3f position = math::Vec3f(pos.x(), pos.y(), pos.y() + transform->size.y() * 0.5f);
                                auto mvparams = std::make_tuple(position);
                                std::vector<EntityId> wallId(1);
                                wallId[0] = clonedWall;

                                componentMapping.template apply<TransformComponent>(moveSystem, wallId, mvparams);

                                addEntity(wallId[0]);
                                gridMap->getGridCellAt(math::Vec3f(transform->position.x(), transform->position.y(), 0))->setPassable(false);

                            } else if (x == endX - tileSize.x() && y == startY && walls.size() >= 11) {
                                EntityId wall = walls[WallType::TOP_RIGHT];
                                EntityId clonedWall;
                                clonedWall = componentMapping.clone<EntityInfoComponent, TransformComponent, WallTypeComponent, MeshComponent, ShadowInfoComponent>(wall);
                                MoveSystem moveSystem;
                                auto transform = componentMapping.getComponent<TransformComponent>(clonedWall);
                                math::Vec3f position = math::Vec3f(pos.x(), pos.y(), pos.y() + transform->size.y() * 0.5f);
                                auto mvparams = std::make_tuple(position);
                                std::vector<EntityId> wallId(1);
                                wallId[0] = clonedWall;
                                componentMapping.template apply<TransformComponent>(moveSystem, wallId, mvparams);
                                addEntity(wallId[0]);
                                gridMap->getGridCellAt(math::Vec3f(transform->position.x(), transform->position.y(), 0))->setPassable(false);
                                //Mur du coin en bas \E0 droite.
                            } else if (x == endX - tileSize.x() && y == endY - tileSize.y() && walls.size() >= 11) {
                                EntityId wall = walls[WallType::BOTTOM_RIGHT];
                                EntityId clonedWall;
                                clonedWall = componentMapping.clone<EntityInfoComponent, TransformComponent, WallTypeComponent, MeshComponent, ShadowInfoComponent>(wall);
                                MoveSystem moveSystem;
                                auto transform = componentMapping.getComponent<TransformComponent>(clonedWall);
                                math::Vec3f position = math::Vec3f(pos.x(), pos.y(), pos.y() + transform->size.y() * 0.5f);
                                auto mvparams = std::make_tuple(position);
                                std::vector<EntityId> wallId(1);
                                wallId[0] = clonedWall;
                                componentMapping.template apply<TransformComponent>(moveSystem, wallId, mvparams);
                                addEntity(wallId[0]);
                                gridMap->getGridCellAt(math::Vec3f(transform->position.x(), transform->position.y(), 0))->setPassable(false);
                            } else if (x == startX && y == endY - tileSize.y() && walls.size() >= 11) {
                                EntityId wall = walls[WallType::BOTTOM_LEFT];
                                EntityId clonedWall;
                                clonedWall = componentMapping.clone<EntityInfoComponent, TransformComponent, WallTypeComponent, MeshComponent, ShadowInfoComponent>(wall);
                                MoveSystem moveSystem;
                                auto transform = componentMapping.getComponent<TransformComponent>(clonedWall);
                                math::Vec3f position = math::Vec3f(pos.x(), pos.y(), pos.y() + transform->size.y() * 0.5f);
                                auto mvparams = std::make_tuple(position);
                                std::vector<EntityId> wallId(1);
                                wallId[0] = clonedWall;
                                componentMapping.template apply<TransformComponent>(moveSystem, wallId, mvparams);
                                addEntity(wallId[0]);
                                gridMap->getGridCellAt(math::Vec3f(transform->position.x(), transform->position.y(), 0))->setPassable(false);
                            } else if ((y == startY || y == endY - tileSize.y()) && walls.size() >= 11) {
                                EntityId wall = walls[WallType::TOP_BOTTOM];
                                EntityId clonedWall;
                                clonedWall = componentMapping.clone<EntityInfoComponent, TransformComponent, WallTypeComponent, MeshComponent, ShadowInfoComponent>(wall);
                                MoveSystem moveSystem;
                                auto transform = componentMapping.getComponent<TransformComponent>(clonedWall);
                                math::Vec3f position = math::Vec3f(pos.x(), pos.y(), pos.y() + transform->size.y() * 0.5f);
                                auto mvparams = std::make_tuple(position);
                                std::vector<EntityId> wallId(1);
                                wallId[0] = clonedWall;
                                componentMapping.template apply<TransformComponent>(moveSystem, wallId, mvparams);
                                addEntity(wallId[0]);
                                gridMap->getGridCellAt(math::Vec3f(transform->position.x(), transform->position.y(), 0))->setPassable(false);
                            } else if ((x == startX || x == endX - tileSize.x()) && walls.size() >= 11) {
                                EntityId wall = walls[WallType::RIGHT_LEFT];
                                EntityId clonedWall;
                                clonedWall = componentMapping.clone<EntityInfoComponent, TransformComponent, WallTypeComponent, MeshComponent, ShadowInfoComponent>(wall);
                                MoveSystem moveSystem;
                                auto transform = componentMapping.getComponent<TransformComponent>(clonedWall);
                                math::Vec3f position = math::Vec3f(pos.x(), pos.y(), pos.y() + transform->size.y() * 0.5f);
                                auto mvparams = std::make_tuple(position);
                                std::vector<EntityId> wallId(1);
                                wallId[0] = clonedWall;
                                componentMapping.template apply<TransformComponent>(moveSystem, wallId, mvparams);
                                addEntity(wallId[0]);
                                gridMap->getGridCellAt(math::Vec3f(transform->position.x(), transform->position.y(), 0))->setPassable(false);
                            } else {
                                EntityId tile;
                                if (tGround.size() > 0)  {
                                    int i = math::Math::random(tGround.size());
                                    EntityId clonedTile;
                                    clonedTile = componentMapping.clone<EntityInfoComponent, TransformComponent, MeshComponent>(tGround[i]);
                                    MoveSystem moveSystem;
                                    auto transform = componentMapping.getComponent<TransformComponent>(clonedTile);
                                    math::Vec3f position = math::Vec3f(pos.x(), pos.y(), pos.y() + transform->size.y() * 0.5f);
                                    auto mvparams = std::make_tuple(position);
                                    std::vector<EntityId> tileId(1);
                                    tileId[0] = clonedTile;
                                    componentMapping.template apply<TransformComponent>(moveSystem, tileId, mvparams);
                                    tile = tileId[0];
                                    Cell* cell = getGridCellAt(math::Vec3f(transform->center.x(), transform->center.y(), 0));
                                    /*for (unsigned l = 0; l < cell->getEntitiesInside().size(); l++) {
                                        if (componentMapping.getComponent<EntityInfoComponent>(cell->getEntitiesInside()[l])->groupName == "E_TILE") {
                                            ////std::cout<<"entities : "<<tile<<" "<<cell->getEntitiesInside()[l]<<std::endl;
                                            for (unsigned int n = 0; n < componentMapping.getComponent<MeshComponent>(cell->getEntitiesInside()[l])->faces.size(); n++) {
                                               ////std::cout<<"adress : "<<&componentMapping.getComponent<MeshComponent>(cell->getEntitiesInside()[l])->faces[n].getTransformMatrix()<<std::endl;
                                               ////std::cout<<"transform matrix : "<<componentMapping.getComponent<MeshComponent>(cell->getEntitiesInside()[l])->faces[n].getTransformMatrix().getMatrix()<<std::endl;

                                            }
                                        }
                                    }*/
                                }
                                componentMapping.addChild(bt, tile);
                            }
                        }
                    }
                    addEntity(bt);
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
            bool Scene::addEntity(EntityId entity) {
                /*if (componentMapping.getComponent<MeshComponent>(entity) && componentMapping.getComponent<EntityInfoComponent>(componentMapping.getRoot(entity)))
                    ////std::cout<<"add wall"<<std::endl;*/
                if (componentMapping.getComponent<AnimationComponent>(entity)) {
                    addEntity(componentMapping.getComponent<AnimationComponent>(entity)->interpolatedFrame);
                } else {

                    std::vector<EntityId> children = componentMapping.getChildren(entity);
                    for (unsigned int i = 0; i < children.size(); i++) {
                         addEntity(children[i]);
                    }
                }
                if (componentMapping.getComponent<MeshComponent>(entity) != nullptr) {
                    for (unsigned int j = 0; j < componentMapping.getComponent<MeshComponent>(entity)->faces.size(); j++) {
                         if (componentMapping.getComponent<MeshComponent>(entity)->faces[j].getMaterial().getTexture() != nullptr) {
                             increaseComptImg(componentMapping.getComponent<MeshComponent>(entity)->faces[j].getMaterial().getTexture());
                         }
                    }
                }
                /*if(componentMapping.getComponent<MeshComponent>(entity) != nullptr && componentMapping.getComponent<TransformComponent>(entity) != nullptr && componentMapping.getComponent<EntityInfoComponent>(entity) != nullptr && componentMapping.getComponent<EntityInfoComponent>(componentMapping.getRoot(entity))->groupName == "E_WALL")
                    ////std::cout<<"add tile wall"<<std::endl;*/
                if (componentMapping.getComponent<TransformComponent>(entity) != nullptr) {
                    /*if (componentMapping.getComponent<MeshComponent>(entity) != nullptr && componentMapping.getComponent<EntityInfoComponent>(componentMapping.getRoot(entity))->groupName == "E_WALL")
                        ////std::cout<<"add wall tile!"<<std::endl;*/
                    if (!gridMap->addEntity(componentMapping.getComponent<TransformComponent>(entity)->globalBounds, entity))
                        return false;
                } else {
                    return false;
                }
                return true;
            }
            bool Scene::removeEntity (EntityId entity) {
                if (componentMapping.getComponent<AnimationComponent>(entity)) {
                    removeEntity(componentMapping.getComponent<AnimationComponent>(entity)->interpolatedFrame);
                } else {
                    std::vector<EntityId> children = componentMapping.getChildren(entity);
                    for (unsigned int i = 0; i < children.size(); i++) {
                        removeEntity(children[i]);
                    }
                }
                if (componentMapping.getComponent<MeshComponent>(entity) != nullptr) {
                    std::vector<Face> faces = componentMapping.getComponent<MeshComponent>(entity)->faces;
                    for (unsigned int j = 0; j < faces.size(); j++) {
                        decreaseComptImg(faces[j].getMaterial().getTexture());
                    }
                }
                return gridMap->removeEntity(componentMapping.getComponent<TransformComponent>(entity)->globalBounds, entity);
            }
            bool Scene::deleteEntity (EntityId entity) {
                if (componentMapping.getComponent<AnimationComponent>(entity)) {
                    deleteEntity(componentMapping.getComponent<AnimationComponent>(entity)->interpolatedFrame);
                } else {
                    std::vector<EntityId> children = componentMapping.getChildren(entity);
                    for (unsigned int i = 0; i < children.size(); i++) {
                        deleteEntity(children[i]);
                    }
                }
                if (componentMapping.getParent(entity) != entt::null) {
                    //////std::cout<<"remove entity : "<<entity->getType()<<std::endl;
                    gridMap->removeEntity(componentMapping.getComponent<TransformComponent>(entity)->globalBounds, entity);
                }
                std::vector<Face> faces = componentMapping.getComponent<MeshComponent>(entity)->faces;
                for (unsigned int j = 0; j < faces.size(); j++) {
                    decreaseComptImg(faces[j].getMaterial().getTexture());
                }
                if (componentMapping.getParent(entity) == entt::null) {
                    return gridMap->removeEntity(componentMapping.getComponent<TransformComponent>(entity)->globalBounds, entity);
                }
                return false;
            }
            math::Vec3f Scene::getPosition() {
                return gridMap->getMins();
            }
            int Scene::getWidth() {
                return gridMap->getSize().x();
            }
            int Scene::getHeight() {
                return gridMap->getSize().y();
            }
            int Scene::getNbCasesPerRow () {
                return gridMap->getNbCellsPerRow();
            }
            void Scene::rotateEntity(EntityId entity, int angle) {
                removeEntity(entity);
                RotationSystem rotationSystem;
                auto rtparams = std::make_tuple(angle);
                std::vector<EntityId> ent;
                ent.push_back(entity);
                componentMapping.template apply<TransformComponent>(rotationSystem, ent, rtparams);
                addEntity(entity);
            }
            void Scene::scaleEntity(EntityId entity, float sx, float sy, float sz) {
                removeEntity(entity);
                ResizeSystem resizeSystem;
                auto rsparams = std::make_tuple(math::Vec3f(sx, sy, sz));
                std::vector<EntityId> ent;
                ent.push_back(entity);
                componentMapping.template apply<TransformComponent>(resizeSystem, ent, rsparams);
                addEntity(entity);
            }
            void Scene::moveEntity(EntityId entity, float dx, float dy, float dz) {
                removeEntity(entity);
                MoveSystem moveSystem;
                auto mvparams = std::make_tuple(math::Vec3f(dx, dy, dz));
                std::vector<EntityId> ent;
                ent.push_back(entity);
                componentMapping.template apply<TransformComponent>(moveSystem, ent, mvparams);
                addEntity(entity);
            }
            void Scene::checkVisibleEntities() {
                for (unsigned int c = 0; c < frcm->getNbECSComponents() + 1; c++) {
                    if (c == frcm->getNbECSComponents() || c < frcm->getNbECSComponents() && frcm->getECSComponent(c) != nullptr) {
                        physic::BoundingBox view;
                        if (c == frcm->getNbECSComponents())
                            view = frcm->getWindow().getView().getViewVolume();
                        else
                            view = frcm->getECSComponent(c)->getView().getViewVolume();
                        visibleEntities.clear();
                        //visibleEntities.resize(core::Application::app->getNbEntitiesTypes());
                        visibleEntities.resize(componentMapping.getEntityFactory().getNbEntitiesTypes());
                        for (unsigned int i = 0; i < visibleEntities.size(); i++) {
                            //visibleEntities[i].resize(core::Application::app->getNbEntities(), nullptr);
                            visibleEntities[i].resize(componentMapping.getEntityFactory().getNbEntities(), entt::null);
                            //////std::cout<<"vector "<<i<<" resized : "<<factory.getNbEntities()<<std::endl;
                        }
                        int x = view.getPosition().x();
                        int y = view.getPosition().y();
                        int z = view.getPosition().z();
                        int endX = view.getPosition().x() + view.getWidth();
                        int endY = view.getPosition().y() + view.getHeight()+100;
                        int endZ = (gridMap->getCellDepth() > 0) ? view.getPosition().z() + view.getDepth()+100 : z;
                        physic::BoundingBox bx (x, y, z, endX-view.getPosition().x(), endY-view.getPosition().y(), endZ-view.getPosition().z());

                        for (int i = x; i <= endX; i+=gridMap->getOffsetX()) {
                            for (int j = y; j <= endY; j+=gridMap->getOffsetY()) {
                                for (int k = z; k <= endZ; k+=gridMap->getOffsetZ()) {
                                    math::Vec3f point(i, j, k);
                                    Cell* cell = getGridCellAt(point);
                                    if (cell != nullptr) {

                                        for (unsigned int n = 0; n < cell->getNbEntitiesInside(); n++) {
                                           EntityId entity = cell->getEntityInside(n);

                                           if (componentMapping.getComponent<EntityInfoComponent>(componentMapping.getRoot(entity)) != nullptr
                                               && componentMapping.getComponent<EntityInfoComponent>(entity) != nullptr && visibleEntities[componentMapping.getEntityFactory().getIntOfType(componentMapping.getComponent<EntityInfoComponent>(componentMapping.getRoot(entity))->groupName)][componentMapping.getComponent<EntityInfoComponent>(entity)->id] == entt::null) {


                                               visibleEntities[componentMapping.getEntityFactory().getIntOfType(componentMapping.getComponent<EntityInfoComponent>(componentMapping.getRoot(entity))->groupName)][componentMapping.getComponent<EntityInfoComponent>(entity)->id] = entity;
                                           }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if (c < frcm->getNbECSComponents() && frcm->getECSComponent(c) != nullptr) {
                        std::vector<EntityId> entities = getVisibleEntities(frcm->getECSComponent(c)->getExpression());
                        frcm->getECSComponent(c)->loadEntitiesOnComponent(componentMapping, entities);
                    }
                }
            }
            void Scene::setBaseChangementMatrix (BaseChangementMatrix bm) {
                gridMap->setBaseChangementMatrix(bm);
            }
            vector<Cell*> Scene::getCasesMap() {
                return gridMap->getCells();
            }

            void Scene::getChildren (EntityId entity, std::vector<EntityId>& entities, std::string type) {
                vector<EntityId> children = componentMapping.getChildren(entity);
                for (unsigned int i = 0; i < children.size(); i++) {
                    if (componentMapping.getChildren(children[i]).size() != 0)
                        getChildren(children[i], children, type);
                    if (type.size() > 0 && type.at(0) == '*') {
                        std::string types;
                        if (type.find("-") != string::npos)
                            types = type.substr(2, type.size() - 3);
                        vector<string> excl = core::split(types, "-");
                        bool exclude = false;
                        for (unsigned int j = 0; j < excl.size(); j++) {
                            if (componentMapping.getComponent<EntityInfoComponent>(children[i])->groupName == excl[j])
                                exclude = true;
                        }
                        if (!exclude) {
                            entities.push_back(children[i]);
                        }
                    } else {
                       vector<string> types = core::split(type, "+");
                       for (unsigned int j = 0; j < types.size(); j++) {
                            if (componentMapping.getComponent<EntityInfoComponent>(children[i])->groupName == types[j]) {
                                entities.push_back(children[i]);
                            }
                       }
                    }
                }
            }
            EntityId Scene::getEntity(int entity) {
                vector<EntityId> allEntities = gridMap->getEntities();
                for (unsigned int i = 0; i < allEntities.size(); i++) {

                    if(componentMapping.getComponent<EntityInfoComponent>(allEntities[i])->id == entity) {

                        return allEntities[i];
                    }
                }
                return entt::null;
            }
            vector<EntityId> Scene::getEntities(string type) {
                vector<EntityId> entities;
                vector<EntityId> allEntities = gridMap->getEntities();
                if (type.size() > 0 && type.at(0) == '*') {
                    if (type.find("-") != string::npos)
                        type = type.substr(2, type.size() - 3);
                    vector<string> excl = core::split(type, "-");
                    for (unsigned int i = 0; i < allEntities.size(); i++) {
                        EntityId entity = allEntities[i];
                        bool exclude = false;
                        for (unsigned int j = 0; j < excl.size(); j++) {
                            if (componentMapping.getComponent<EntityInfoComponent>(entity)->groupName == excl[j])
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
                                UpdateTransformSystem uts;
                                auto params = std::make_tuple();
                                std::vector<EntityId> ents;
                                ents.push_back(entity);
                                componentMapping.apply<TransformComponent>(uts, ents, params);
                                entities.push_back(ents[0]);
                            }
                        }
                    }
                    return entities;
                }
                vector<string> types = core::split(type, "+");
                for (unsigned int i = 0; i < types.size(); i++) {
                    for (unsigned int j = 0; j < allEntities.size(); j++) {
                        EntityId entity = allEntities[j];
                        if (componentMapping.getComponent<EntityInfoComponent>(entity)->groupName == types[i]) {
                            bool contains = false;
                            for (unsigned int n = 0; n < entities.size() && !contains; n++) {
                                if (entities[n] == entity) {
                                    contains = true;
                                }
                            }
                            if (!contains) {
                                UpdateTransformSystem uts;
                                auto params = std::make_tuple();
                                std::vector<EntityId> ents;
                                ents.push_back(entity);
                                componentMapping.apply<TransformComponent>(uts, ents, params);
                                entities.push_back(ents[0]);
                            }
                        }
                    }
                }
                return entities;
            }
            vector<EntityId> Scene::getRootEntities(string type) {
                vector<EntityId> entities;
                vector<EntityId> allEntities = gridMap->getEntities();
                if (type.size() > 0 && type.at(0) == '*') {
                    if (type.find("-") != string::npos)
                        type = type.substr(2, type.size() - 3);
                    vector<string> excl = core::split(type, "-");
                    for (unsigned int i = 0; i < allEntities.size(); i++) {
                        EntityId entity = componentMapping.getRoot(allEntities[i]);
                        bool exclude = false;
                        for (unsigned int j = 0; j < excl.size(); j++) {
                            if (componentMapping.getComponent<EntityInfoComponent>(componentMapping.getRoot(entity))->groupName == excl[j])
                                exclude = true;
                        }
                        if (!exclude) {
                            bool contains = false;
                            for (unsigned int n = 0; n < entities.size() && !contains; n++) {
                                if (componentMapping.getRoot(entities[n])  == componentMapping.getRoot(entity)) {
                                    contains = true;
                                }
                            }
                            if (!contains) {
                                UpdateTransformSystem uts;
                                auto params = std::make_tuple();
                                std::vector<EntityId> ents;
                                ents.push_back(componentMapping.getRoot(entity));
                                componentMapping.apply<TransformComponent>(uts, ents, params);
                                entities.push_back(ents[0]);
                            }
                        }
                    }
                    return entities;
                }
                vector<string> types = core::split(type, "+");
                for (unsigned int i = 0; i < types.size(); i++) {
                    for (unsigned int j = 0; j < allEntities.size(); j++) {
                        EntityId entity = componentMapping.getRoot(allEntities[j]);
                        if (componentMapping.getComponent<EntityInfoComponent>(componentMapping.getRoot(entity))->groupName == types[i]) {
                            bool contains = false;
                            for (unsigned int n = 0; n < entities.size() && !contains; n++) {
                                if (componentMapping.getRoot(entities[n]) == componentMapping.getRoot(entity)) {
                                    contains = true;
                                }
                            }
                            if (!contains) {
                                UpdateTransformSystem uts;
                                auto params = std::make_tuple();
                                std::vector<EntityId> ents;
                                ents.push_back(componentMapping.getRoot(entity));
                                componentMapping.apply<TransformComponent>(uts, ents, params);
                                entities.push_back(ents[0]);
                            }
                        }
                    }
                }
                return entities;
            }
            vector<EntityId> Scene::getChildrenEntities(string type) {
                vector<EntityId> entities;
                vector<EntityId> allEntities = gridMap->getEntities();
                if (type.size() > 0 && type.at(0) == '*') {
                    if (type.find("-") != string::npos)
                        type = type.substr(2, type.size() - 3);
                    vector<string> excl = core::split(type, "-");
                    for (unsigned int i = 0; i < allEntities.size(); i++) {
                        EntityId entity = allEntities[i];
                        bool exclude = false;
                        for (unsigned int j = 0; j < excl.size(); j++) {
                            if (componentMapping.getComponent<EntityInfoComponent>(componentMapping.getRoot(entity))->groupName == excl[i])
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
                                UpdateTransformSystem uts;
                                auto params = std::make_tuple();
                                std::vector<EntityId> ents;
                                ents.push_back(componentMapping.getRoot(entity));
                                componentMapping.apply<TransformComponent>(uts, ents, params);
                                entities.push_back(ents[0]);
                            }
                        }
                    }
                    return entities;
                }
                vector<string> types = core::split(type, "+");
                for (unsigned int i = 0; i < types.size(); i++) {
                    for (unsigned int j = 0; j < allEntities.size(); j++) {
                        EntityId entity = allEntities[j];
                        if (componentMapping.getComponent<EntityInfoComponent>(componentMapping.getRoot(entity))->groupName == types[i]) {
                            bool contains = false;
                            for (unsigned int n = 0; n < entities.size() && !contains; n++) {
                                if (entities[n] == entity) {
                                    contains = true;
                                }
                            }
                            if (!contains) {
                                UpdateTransformSystem uts;
                                auto params = std::make_tuple();
                                std::vector<EntityId> ents;
                                ents.push_back(componentMapping.getRoot(entity));
                                componentMapping.apply<TransformComponent>(uts, ents, params);
                                entities.push_back(ents[0]);
                            }
                        }
                    }
                }
                return entities;
            }

            vector<EntityId> Scene::getVisibleEntities (std::string type) {
                std::vector<EntityId> entities;
                if (type.size() > 0 && type.at(0) == '*') {
                    if (type.find("-") != string::npos)
                        type = type.substr(2, type.size() - 2);
                    vector<string> excl = core::split(type, "-");
                    for (unsigned int i = 0; i < visibleEntities.size(); i++) {
                        for (unsigned int j = 0; j < visibleEntities[i].size(); j++) {
                            if (visibleEntities[i][j] != entt::null) {
                                bool exclude = false;
                                for (unsigned int t = 0; t < excl.size(); t++) {
                                    if (componentMapping.getComponent<EntityInfoComponent>(componentMapping.getRoot(visibleEntities[i][j]))->groupName == excl[t])
                                        exclude = true;
                                }
                                if (!exclude) {
                                    EntityId ba = componentMapping.getRoot(visibleEntities[i][j]);
                                    if (componentMapping.getComponent<SelectedAnimationComponent>(ba) != nullptr) {
                                        if (componentMapping.getComponent<SelectedAnimationComponent>(ba)->selectedAnimIndex == componentMapping.getComponent<EntityInfoComponent>(visibleEntities[i][j])->animIndex) {
                                            entities.push_back(visibleEntities[i][j]);
                                        }
                                    } else {
                                        entities.push_back(visibleEntities[i][j]);
                                    }
                                }
                            }
                        }
                    }
                    return entities;
                }
                //////std::cout<<"get visible entities"<<std::endl;
                vector<string> types = core::split(type, "+");
                for (unsigned int t = 0; t < types.size(); t++) {
                    //unsigned int type = core::Application::app->getIntOfType(types[t]);
                    unsigned int type = componentMapping.getEntityFactory().getIntOfType(types[t]);
                    if (type < visibleEntities.size()) {
                        vector<EntityId> visibleEntitiesType = visibleEntities[type];
                        for (unsigned int i = 0; i < visibleEntitiesType.size(); i++) {
                            bool found = false;
                            for (unsigned int j = 0; j < types.size(); j++) {
                                if (visibleEntitiesType[i] != entt::null && componentMapping.getComponent<EntityInfoComponent>(componentMapping.getRoot(visibleEntitiesType[i]))->groupName == types[j]) {
                                    found = true;
                                }
                            }
                            if (visibleEntitiesType[i] != entt::null && found) {
                                EntityId ba = componentMapping.getRoot(visibleEntitiesType[i]);
                                if (componentMapping.getComponent<SelectedAnimationComponent>(ba) != nullptr) {
                                        if (componentMapping.getComponent<SelectedAnimationComponent>(ba)->selectedAnimIndex == componentMapping.getComponent<EntityInfoComponent>(visibleEntitiesType[i])->animIndex) {
                                            entities.push_back(visibleEntitiesType[i]);
                                        }
                                } else {


                                    /*if (componentMapping.getComponent<MeshComponent>(visibleEntitiesType[i]) != nullptr && componentMapping.getComponent<EntityInfoComponent>(cm.getRoot(visibleEntitiesType[i]))->groupName == "E_WALL")
                                        ////std::cout<<"get visible entity add wall"<<std::endl;*/
                                    entities.push_back(visibleEntitiesType[i]);
                                }
                            }
                        }
                    }
                }
                return entities;
            }

            vector<EntityId> Scene::getEntitiesInBox (physic::BoundingBox bx, std::string type) {
                 vector<EntityId> entities;
                 vector<EntityId> allEntitiesInRect = gridMap->getEntitiesInBox(componentMapping, bx);

                 if (type.at(0) == '*') {
                    if (type.find("-") != string::npos)
                        type = type.substr(2, type.size() - 3);
                    vector<string> excl = core::split(type, "-");
                    for (unsigned int i = 0; i < allEntitiesInRect.size(); i++) {
                        EntityId entity = allEntitiesInRect[i];
                        if (entity != entt::null) {
                            bool exclude = false;
                            for (unsigned int i = 0; i < excl.size(); i++) {
                                if (componentMapping.getComponent<EntityInfoComponent>(componentMapping.getRoot(entity))->groupName == excl[i])
                                    exclude = true;
                            }
                            if (!exclude) {
                                EntityId ba = componentMapping.getRoot(entity);
                                if (componentMapping.getComponent<SelectedAnimationComponent>(ba) != nullptr) {
                                    if (componentMapping.getComponent<SelectedAnimationComponent>(ba)->selectedAnimIndex == componentMapping.getComponent<EntityInfoComponent>(entity)->animIndex) {
                                        entities.push_back(entity);
                                    }
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
                    for (unsigned int j = 0; j < allEntitiesInRect.size(); j++) {
                        EntityId entity = allEntitiesInRect[j];
                        if (entity != entt::null) {
                            if (componentMapping.getComponent<EntityInfoComponent>(componentMapping.getRoot(entity))->groupName == types[i]) {
                                EntityId ba = componentMapping.getRoot(entity);
                                if (componentMapping.getComponent<SelectedAnimationComponent>(ba) != nullptr) {
                                    if (componentMapping.getComponent<SelectedAnimationComponent>(ba)->selectedAnimIndex == componentMapping.getComponent<EntityInfoComponent>(entity)->animIndex) {
                                        entities.push_back(entity);
                                    }
                                } else {
                                    entities.push_back(entity);
                                }
                            }
                        }
                    }
                }
                return entities;
            }
            math::Vec3f Scene::getCoordinatesAt(math::Vec3f p) {
                math::Vec3f c(p.x(), p.y(), p.z());
                return gridMap->getCoordinatesAt(c);
            }
            Cell* Scene::getGridCellAt(math::Vec3f p) {
                return gridMap->getGridCellAt(p);
            }
            vector<Cell*> Scene::getCasesInBox (physic::BoundingBox bx) {
                return gridMap->getCellsInBox(bx);
            }
            bool Scene::collide (EntityId entity, math::Vec3f position) {
                 return gridMap->collideWithEntity(componentMapping, entity, position);
            }
            bool Scene::collide (EntityId entity) {
                 return gridMap->collideWithEntity(componentMapping, entity);
            }
            bool Scene::collide (EntityId entity, math::Ray ray) {
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

            void Scene::drawOnComponents(std::string expression, int layer, BlendMode blendMode) {
                Component* frc = frcm->getECSComponent(layer);
                if (frc != nullptr) {
                    frc->setExpression(expression);
                }
            }
            void Scene::drawOnComponents(Drawable& drawable, int layer, RenderStates states) {
                Component *frc = frcm->getECSComponent(layer);
                if (frc != nullptr) {
                    frc->draw(drawable, states);
                }
            }
            BaseChangementMatrix Scene::getBaseChangementMatrix() {
                return gridMap->getBaseChangementMatrix();
            }
        }
    }
}
