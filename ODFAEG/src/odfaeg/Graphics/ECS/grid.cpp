#include "../../../../include/odfaeg/Graphics/ECS/grid.hpp"
#include "../../../../include/odfaeg/Physics/boundingBox.h"
using namespace std;
namespace odfaeg {
    namespace graphic {
        namespace ecs {
            Grid::Grid (int cellWidth, int cellHeight, int cellDepth) {
                nbCellsPerRow = 0;
                nbCellsPerCol = 0;
                minX = minY = minZ = std::numeric_limits<int>::max();
                maxX = maxY = maxZ = std::numeric_limits<int>::min();
                this->cellWidth = cellWidth;
                this->cellHeight = cellHeight;
                this->cellDepth = cellDepth;
                offsetX = (cellWidth > 0) ? cellWidth * 0.5f : 1;
                offsetY = (cellHeight > 0) ? cellHeight * 0.5f : 1;
                offsetZ = (cellDepth > 0) ? cellDepth * 0.5f : 1;

            }

            int Grid::getCellWidth() {
                return cellWidth;
            }

            int Grid::getCellHeight() {
                return cellHeight;
            }
            int Grid::getCellDepth() {
                return cellDepth;
            }
            int Grid::getNbCellsPerRow () {
                return nbCellsPerRow;
            }
            int Grid::getNbCellsPerCol () {
                return nbCellsPerCol;
            }
            void Grid::setBaseChangementMatrix(BaseChangementMatrix bm) {
                this->bm = bm;
            }

            BaseChangementMatrix Grid::getBaseChangementMatrix() {
                return bm;
            }
            bool Grid::addEntity (physic::BoundingBox globalBounds, EntityId entity){
                int x = globalBounds.getPosition().x;
                int y = globalBounds.getPosition().y;
                int z = globalBounds.getPosition().z;
                int endX = (x + globalBounds.getWidth());
                int endY = (y + globalBounds.getHeight());
                int endZ = (z + globalBounds.getDepth());
                bool added = false;
                //std::cout<<"global bounds : "<<globalBounds.getPosition()<<globalBounds.getSize()<<std::endl;
                /*std::array<math::Vec2f, 4> pos;
                pos[0] = math::Vec2f(x, y);
                pos[1] = math::Vec2f(x, y + endY);
                pos[2] = math::Vec2f(x + endX, y + endY);
                pos[3] = math::Vec2f(x + endX, y);

                for (unsigned int i = 0; i < pos.size(); i++) {
                    if (!(containsEntity(entity, pos[i]))) {
                        Cell *cm = getGridCellAt(pos[i]);
                        if (cm == nullptr) {
                            createCell(pos[i]);
                            cm = getGridCellAt(pos[i]);
                        }
                        added = true;
                        cm->addEntity(entity);
                    }
                }*/
                //std::cout<<"offsets : "<<offsetX<<","<<offsetY<<","<<offsetZ<<std::endl<<"start : "<<x<<","<<y<<","<<z<<std::endl<<"ends : "<<endX<<","<<endY<<","<<endZ<<std::endl;
                //std::cout<<"add entity : "<<entity->getType()<<std::endl<<x<<","<<y<<","<<z<<","<<endX<<","<<endY<<","<<endZ<<std::endl;
                for (int i = x; i <= endX; i+= offsetX) {
                    for (int j = y; j <= endY; j+= offsetY)  {
                        for (int k = z; k <= endZ; k+= offsetZ) {

                            math::Vec3f pos (i, j, k);

                            if (!(containsEntity(entity, pos))) {
                                //std::cout<<"get cell map"<<std::endl;
                                Cell *cm = getGridCellAt(pos);
                                //std::cout<<"create cell map"<<std::endl;
                                if (cm == nullptr) {
                                    //std::cout<<"create cell map"<<std::endl;
                                    createCell(pos);
                                    cm = getGridCellAt(pos);
                                }
                                added = true;
                                cm->addEntity(entity);
                                /*if (entity->getType() == "E_BIGTILE")
                                  std::cout<<cm->getCoords()<<std::endl;*/
                                //std::cout<<"entity added"<<std::endl;
                                /*if (entity->getRootType() == "E_WALL") {
                                    int indice = (math::Math::abs(minX) + cm->getCoords().x)
                                    + (math::Math::abs(minY) + cm->getCoords().y) * nbCellsPerRow + (math::Math::abs(minZ) + cm->getCoords().z) * nbCellsPerCol;
                                    std::cout<<"add wall at : "<<pos<<cm->getCoords()<<minX<<std::endl<<"miny : "<<minY<<std::endl<<"minz : "<<minZ<<std::endl<<"nb Cells per row : "<<nbCellsPerRow<<std::endl<<"nb Cells per col : "<<nbCellsPerCol<<std::endl<<"index : "<<indice<<std::endl;
                                }*/
                                /*if (i == x && j == y && k == z && entity->getType() == "E_TILE") {*/

                                    /*int indice = (math::Math::abs(minX) + cm->getCoords().x)
                                                        + (math::Math::abs(minY) + cm->getCoords().y) * nbCellsPerRow + (math::Math::abs(minZ) + cm->getCoords().z) * nbCellsPerCol;
                                    std::cout<<"add entity mins : "<<pos<<std::endl<<minX<<","<<minY<<","<<minZ<<std::endl<<"maxs : "<<maxX<<","<<maxY<<","<<maxZ<<std::endl<<"nb Cells : "<<nbCellsPerRow<<","<<nbCellsPerCol<<std::endl<<"coords : "<<cm->getCoords()<<std::endl;*/



                                    //system("PAUSE");
                                //}

                            }
                        }
                        //std::cout<<"leave k"<<std::endl;
                    }
                    //std::cout<<"leave j"<<std::endl;
                }
                //std::cout<<"entity added : "<<std::endl;
                return added;
            }

            bool Grid::containsEntity(EntityId entity, math::Vec3f pos) {
                Cell *cell = getGridCellAt(pos);
                if (cell !=nullptr) {
                     if (cell->containsEntity(entity)) {
                         return true;
                     }
                }
                return false;
            }

            EntityId Grid::getEntity (ComponentMapping mapping, int id) {
                for (unsigned int i = 0; i < cells.size(); i++) {
                    Cell *cm = cells[i];
                    if (cm != nullptr) {
                        for (unsigned int j = 0; j < cm->getEntitiesInside().size(); j++) {
                            EntityId entity = cm->getEntityInside(j);
                            if (entity == id) {
                                return entity;
                            }
                        }
                    }
                }
            }
            /*EntityId Grid::getEntity (std::string name) {
                for (unsigned int i = 0; i < cells.size(); i++) {
                    Cell *cm = cells[i];
                    if (cm != nullptr) {
                        for (unsigned int j = 0; j < cm->getEntitiesInside().size(); j++) {
                            Entity *entity = cm->getEntityInside(j);
                            if (entity->getName() == name) {
                                return entity;
                            }
                            Entity* parent = entity->getParent();
                            while (parent != nullptr) {
                                if (parent->getName() == name) {
                                    return parent;
                                }
                                parent = parent->getParent();
                            }
                        }
                    }
                }
                return nullptr;
            }*/

            void Grid::createCell (math::Vec3f &point) {
                //std::cout<<"point : "<<point<<std::endl;
                math::Vec3f coordsCaseP = getCoordinatesAt(point);
                //std::cout<<"coords caseP : "<<coordsCaseP<<std::endl;

                /*minX = (coordsCaseP.x < minX) ? coordsCaseP.x : minX;
                minY = (coordsCaseP.y < minY) ? coordsCaseP.y : minY;
                minZ = (coordsCaseP.z < minZ) ? coordsCaseP.z : minZ;
                maxX = (coordsCaseP.x > maxX) ? coordsCaseP.x : maxX;
                maxY = (coordsCaseP.y > maxY) ? coordsCaseP.y : maxY;
                maxZ = (coordsCaseP.z > maxZ) ? coordsCaseP.z : maxZ;*/

                math::Vec3f p = bm.unchangeOfBase(point);

                math::Vec3f v1;
                v1.x = (cellWidth > 0) ? (int) p.x / cellWidth : 0;
                v1.y = (cellHeight > 0) ? (int) p.y / cellHeight : 0;
                v1.z = (cellDepth > 0) ? (int) p.z / cellDepth : 0;
                if (p.x <= 0)
                    v1.x--;
                if (p.y <= 0)
                    v1.y--;
                if (p.z <= 0)
                    v1.z--;
                v1.x *= cellWidth;
                v1.y *= cellHeight;
                v1.z *= cellDepth;
                math::Vec3f v[8];
                v[0] = math::Vec3f (v1.x, v1.y, v1.z);
                v[1] = math::Vec3f (v1.x + cellWidth, v1.y, v1.z);
                v[2] = math::Vec3f (v1.x + cellWidth, v1.y + cellHeight, v1.z);
                v[3] = math::Vec3f (v1.x, v1.y + cellHeight, v1.z);
                v[4] = math::Vec3f (v1.x, v1.y, v1.z+cellDepth);
                v[5] = math::Vec3f (v1.x + cellWidth, v1.y, v1.z+cellDepth);
                v[6] = math::Vec3f (v1.x + cellWidth, v1.y + cellHeight, v1.z+cellDepth);
                v[7] = math::Vec3f (v1.x, v1.y + cellHeight, v1.z+cellDepth);

                for (unsigned int i = 0; i < 8; i++) {
                    v[i] = bm.changeOfBase(v[i]);
                    /*if (i < 4)
                        std::cout<<"point "<<i<<" : "<<v[i]<<std::endl;*/
                }

                //Face de devant.
                physic::BoundingPolyhedron bp(v[0], v[1], v[2], true);
                bp.addTriangle(v[0], v[2], v[3]);
                //Face gauche.
                bp.addTriangle(v[0], v[1], v[7]);
                bp.addTriangle(v[0], v[3], v[7]);
                //Face droite.
                bp.addTriangle(v[1], v[5], v[6]);
                bp.addTriangle(v[1], v[2], v[6]);
                //Face de derrière.
                bp.addTriangle(v[4], v[5], v[6]);
                bp.addTriangle(v[4], v[7], v[6]);
                //Face du dessus.
                bp.addTriangle(v[0], v[4], v[5]);
                bp.addTriangle(v[0], v[1], v[5]);
                //Face du dessous.
                bp.addTriangle(v[3], v[7], v[6]);
                bp.addTriangle(v[3], v[2], v[6]);
                //std::cout<<"center : "<<bp->getCenter()<<std::endl;
                Cell *cell = new Cell(bp, coordsCaseP);
                cells.push_back(cell);
                checkExts();
                cells.pop_back();

                nbCellsPerRow = (cellWidth > 0) ? math::Math::abs(minX) + maxX + 1 : 1;
                nbCellsPerCol = (cellHeight > 0) ? math::Math::abs(minY) + maxY + 1 : 1;
                int nbCellsPerDepth = (cellDepth > 0) ? math::Math::abs(minZ) + maxZ + 1 : 1;
                //std::cout<<"nbCellsPerRow : "<<nbCellsPerRow<<std::endl<<"nbCellsPerCol : "<<nbCellsPerCol<<"nb Cells per depth"<<nbCellsPerDepth<<std::endl;
                unsigned int newSize = nbCellsPerCol * nbCellsPerRow * nbCellsPerDepth;
                //std::cout<<"min z : "<<minZ<<std::endl;
                int indice = (math::Math::abs(minX) + coordsCaseP.x)
                             + (math::Math::abs(minY) + coordsCaseP.y) * nbCellsPerRow + (math::Math::abs(minZ) + coordsCaseP.z) * nbCellsPerCol;
                //std::cout<<"create cell map at indice : "<<indice<<std::endl;
                if (newSize > cells.size()) {
                    //std::cout<<"resize vector! : "<<newSize<<std::endl;
                    vector<Cell*> tmpCells = cells;
                    cells.clear();
                    cells.resize(newSize);
                    std::fill(cells.begin(), cells.end(), nullptr);
                    for (unsigned int i = 0; i < tmpCells.size(); i++) {
                        if (tmpCells[i] != nullptr) {
                            math::Vec3f coords = tmpCells[i]->getCoords();
                            int newInd = (math::Math::abs(minX) + coords.x)
                                         + (math::Math::abs(minY) + coords.y) * nbCellsPerRow + (math::Math::abs(minZ) + coords.z) * nbCellsPerCol;
                            //std::cout<<"new ind  : "<<newInd<<std::endl;
                            cells[newInd] = tmpCells[i];
                        }
                    }
                } else if (newSize < cells.size()) {
                    //std::cout<<"resize vector! : "<<newSize<<std::endl;
                    vector<Cell*> tmpCells = cells;
                    cells.clear();
                    cells.resize(newSize);
                    std::fill(cells.begin(), cells.end(), nullptr);
                    for (unsigned int i = 0; i < tmpCells.size(); i++) {
                        if (tmpCells[i] != nullptr) {
                            math::Vec3f coords = tmpCells[i]->getCoords();
                            int newInd = (math::Math::abs(minX) + coords.x)
                                         + (math::Math::abs(minY) + coords.y) * nbCellsPerRow + (math::Math::abs(minZ) + coords.z) * nbCellsPerCol;
                            //std::cout<<"new ind  : "<<newInd<<std::endl;
                            cells[newInd] = tmpCells[i];
                        }
                    }
                }
                //std::cout<<"ind : "<<indice<<std::endl;
                cells[indice] = cell;
                //system("PAUSE");
            }


            //Supprime une tile dans la cellule. (Sans la supprimer de la mémoire.)
            bool Grid::removeEntity (physic::BoundingBox globalBounds, EntityId entity) {

                int x = globalBounds.getPosition().x;
                int y = globalBounds.getPosition().y;
                int z = globalBounds.getPosition().z;
                int endX = (x + globalBounds.getWidth());
                int endY = (y + globalBounds.getHeight());
                int endZ = (z + globalBounds.getDepth());
                bool removed = false;
                for (int i = x; i <= endX; i+= offsetX) {
                    for (int j = y; j <= endY; j+= offsetY) {
                        for (int k = z; k <= endZ; k+= offsetZ) {
                            math::Vec3f pos (i, j, k);
                            Cell *cm = getGridCellAt(pos);
                            math::Vec3f coords = getCoordinatesAt(pos);
                            int indice = (math::Math::abs(minX) + coords.x)
                                        + (math::Math::abs(minY) + coords.y) * nbCellsPerRow + (math::Math::abs(minZ) + coords.z) * nbCellsPerCol;
                            //std::cout<<"remove entity indice : "<<indice<<std::endl<<"mins : "<<std::endl<<minX<<","<<minY<<","<<minZ<<std::endl<<"maxs : "<<maxX<<","<<maxY<<","<<maxZ<<std::endl<<"nb Cells : "<<nbCellsPerRow<<","<<nbCellsPerCol<<std::endl<<"coords : "<<coords<<"size : "<<cells.size()<<std::endl;
                            if (cm != nullptr) {
                              /*if (i == x && j == y && k == z && entity->getType() == "E_TILE") {
                                  int indice = (math::Math::abs(minX) + cm->getCoords().x)
                                        + (math::Math::abs(minY) + cm->getCoords().y) * nbCellsPerRow + (math::Math::abs(minZ) + cm->getCoords().z) * nbCellsPerCol;
                              }*/
                              if(cm->removeEntity(entity)) {
                                removed = true;
                              }
                              if (!cm->isEntityInside())
                                    removeCell(cm);
                            }
                        }
                    }
                }
                return removed;
            }
            void Grid::removeCell (Cell *cell) {

                for (unsigned int i = 0; i < cells.size(); i++) {
                    if (cells[i] != nullptr && cells[i]==cell) {
                        //std::cout<<"delete cell : "<<cells[i]->getCoords()<<std::endl;
                        delete cells[i];
                        cells[i] = nullptr;
                    }
                }
                //Supprime les Cells vides à la fin du vecteur.
                //On recherche les coordonnées de la case la plus grande.
                checkExts();
                //On cherche si il faut réduire la taille du vecteur. (En partant du début.)
                nbCellsPerRow = (cellWidth > 0) ? math::Math::abs(minX) + maxX + 1 : 1;
                nbCellsPerCol = (cellHeight > 0) ? math::Math::abs(minY) + maxY + 1 : 1;
                int nbCellsPerDepth = (cellDepth > 0) ? math::Math::abs(minZ) + maxZ + 1 : 1;
                unsigned int newSize = nbCellsPerCol * nbCellsPerRow * nbCellsPerDepth;

                if (newSize < cells.size()) {
                    //std::cout<<"new size : "<<newSize<<std::endl;
                    vector<Cell*> tmpcells = cells;
                    cells.clear();
                    cells.resize(newSize);
                    std::fill(cells.begin(), cells.end(), nullptr);
                    for (unsigned int i = 0; i < tmpcells.size(); i++) {
                        if (tmpcells[i] != nullptr) {
                            math::Vec3f coords = tmpcells[i]->getCoords();
                            int newInd = math::Math::abs(minX) + coords.x + (math::Math::abs(minY) + coords.y) * nbCellsPerRow + (math::Math::abs(minZ) + coords.z) * nbCellsPerCol;
                            cells[newInd] = tmpcells[i];
                        }
                    }
                } else if (newSize > cells.size()) {
                    //std::cout<<"new size : "<<newSize<<std::endl;
                    vector<Cell*> tmpcells = cells;
                    cells.clear();
                    cells.resize(newSize);
                    std::fill(cells.begin(), cells.end(), nullptr);
                    for (unsigned int i = 0; i < tmpcells.size(); i++) {
                        if (tmpcells[i] != nullptr) {
                            math::Vec3f coords = tmpcells[i]->getCoords();
                            int newInd = math::Math::abs(minX) + coords.x + (math::Math::abs(minY) + coords.y) * nbCellsPerRow + (math::Math::abs(minZ) + coords.z) * nbCellsPerCol;
                            cells[newInd] = tmpcells[i];
                        }
                    }
                }
            }

            vector<Cell*> Grid::getCellsInBox (physic::BoundingBox bx) {

                vector<Cell*> cells;
                int x = bx.getPosition().x;
                int y = bx.getPosition().y;
                int z = bx.getPosition().z;
                int endX = (x + bx.getWidth());
                int endY = (y + bx.getHeight());
                int endZ = (z + bx.getDepth());
                for (int i = x; i <= endX; i+= offsetX) {
                    for (int j = y; j <= endY; j+= offsetY) {
                        for (int k = 0; k <= endZ; k+= offsetZ) {
                            math::Vec3f p (i, j, k);
                            Cell *cell = getGridCellAt(p);
                            if (cell != nullptr) {
                                bool contains = false;
                                for (unsigned int i = 0; i < cells.size(); i++) {
                                    if (cells[i] == cell)
                                        contains = true;
                                }
                                if (!contains)
                                    cells.push_back(cell);
                            }
                        }
                    }
                }
                return cells;
            }
            vector<Cell*> Grid::getNeightbours(EntityId entity, Cell *cell, bool getCellOnPassable) {
                math::Vec3f coords = cell->getCoords();
                vector<Cell*> neightbours;
                for (int i = coords.x - 1; i <= coords.x + 1; i++) {
                    for (int j = coords.y - 1; j <= coords.y + 1; j++) {
                        for (int k = coords.z - 1; k <= coords.z + 1; k++) {
                            if (!(i == coords.x && j == coords.y && k == coords.z)) {
                                math::Vec2f neightbourCoords(i, j);
                                Cell* neightbour = getGridCellAtFromCoords(neightbourCoords);
                                if (neightbour != nullptr) {
                                    if (getCellOnPassable)
                                        neightbours.push_back(neightbour);
                                    else {
                                        if (getComponent<ColliderComponent>(entity)->boundingVolume != nullptr) {
                                            bool collide = false;
                                            math::Vec3f t = neightbour->getCenter() - getComponent<ColliderComponent>(entity)->boundingVolume->getCenter();
                                            std::unique_ptr<physic::BoundingVolume> cv = getComponent<ColliderComponent>(entity)->boundingVolume->clone();
                                            cv->move(t);
                                            for (unsigned int k = 0; k < neightbour->getEntitiesInside().size() && !collide; k++) {
                                                if (getComponent<ColliderComponent>(neightbour->getEntitiesInside()[k])->boundingVolume != nullptr && neightbour->getEntitiesInside()[k] != entity) {
                                                    physic::CollisionResultSet::Info info;
                                                    if (cv->intersects(*getComponent<ColliderComponent>(neightbour->getEntitiesInside()[k])->boundingVolume, info)) {
                                                        if (cv->getChildren().size() == 0) {
                                                            collide = true;
                                                        }
                                                    }
                                                }
                                            }
                                            if (!collide) {
                                                neightbours.push_back(neightbour);
                                            }
                                        } else {
                                            if (neightbour->isPassable()) {
                                                neightbours.push_back(neightbour);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                return neightbours;
            }
            vector<EntityId> Grid::getEntitiesInBox(ComponentMapping& mapping, physic::BoundingBox box) {
                vector<EntityId> entities;
                int x = box.getPosition().x;
                int y = box.getPosition().y;
                int z = box.getPosition().z;
                int endX = box.getPosition().x + box.getWidth();
                int endY = box.getPosition().y + box.getHeight();
                int endZ = box.getPosition().z + box.getDepth();
                physic::BoundingBox bx (x, y, z, endX-x, endY-y, z - endZ);
                for (int i = x; i <= endX; i+=offsetX) {
                    for (int j = y; j <= endY; j+=offsetY) {
                        for (int k = z; k <= endZ; k+= offsetZ) {
                            math::Vec3f point(i, j, k);
                            Cell* cell = getGridCellAt(point);
                            if (cell != nullptr) {
                                for (unsigned int n = 0; n < cell->getEntitiesInside().size(); n++) {
                                   EntityId entity = cell->getEntityInside(n);
                                   physic::BoundingBox globalBounds = getComponent<TransformComponent>(entity)->globalBounds;
                                   bool contains = false;
                                   for (unsigned int k = 0; k < entities.size() && !contains; k++) {
                                        if (entities[k] == entity)
                                            contains = true;
                                   }
                                   if (!contains && bx.intersects(globalBounds) || bx.isInside(globalBounds) || globalBounds.isInside(bx)) {

                                        entities.push_back(entity);
                                   }
                                }
                            }
                        }
                    }
                }
                return entities;
            }
            bool Grid::collideWithEntity(ComponentMapping& componentMapping, EntityId entity, math::Vec3f position) {
            Cell* cell = getGridCellAt(position);
            if (cell != nullptr) {
                if (!cell->isPassable())
                    return true;
                std::vector<Cell*> neightbours = getNeightbours(entity,cell,true);
                for (unsigned int i = 0; i < neightbours.size(); i++) {
                    if (!neightbours[i]->isPassable()) {
                        return true;
                    }
                }
                if (getComponent<ColliderComponent>(componentMapping.getRoot(entity))->boundingVolume != nullptr) {
                    math::Vec3f t = position - getComponent<ColliderComponent>(componentMapping.getRoot(entity))->boundingVolume->getCenter();
                    physic::BoundingVolume* cv = getComponent<ColliderComponent>(componentMapping.getRoot(entity))->boundingVolume->clone().release();
                    cv->move(t);
                    for (unsigned int k = 0; k < cell->getEntitiesInside().size(); k++)  {
                        if (getComponent<ColliderComponent>(componentMapping.getRoot(cell->getEntitiesInside()[k]))->boundingVolume != nullptr && componentMapping.getRoot(cell->getEntitiesInside()[k]) != entity) {
                            physic::CollisionResultSet::Info info;
                            if (cv->intersects(*getComponent<ColliderComponent>(componentMapping.getRoot(cell->getEntitiesInside()[k]))->boundingVolume, info)) {
                                /*info.entity = cell->getEntitiesInside()[k]->getRootEntity();
                                info.center = cv->getCenter();
                                physic::CollisionResultSet::pushCollisionInfo(info);*/
                                if (cv->getChildren().size() == 0) {
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
            return false;
        }
        bool Grid::collideWithEntity(ComponentMapping& componentMapping, EntityId entity) {
            std::unique_ptr<physic::BoundingVolume> bv1;
            physic::BoundingBox bx = getComponent<TransformComponent>(entity)->globalBounds;
            if (getComponent<ColliderComponent>(entity)->boundingVolume != nullptr) {
                bv1 = getComponent<ColliderComponent>(entity)->boundingVolume->clone();
            }
            std::vector<Cell*> cells = getCellsInBox(bx);
            for (unsigned int i = 0; i < cells.size(); i++) {
                for (unsigned int j = 0; j < cells[i]->getEntitiesInside().size(); j++) {
                    EntityId entity2 = componentMapping.getRoot(cells[i]->getEntitiesInside()[j]);
                    if (entity2 != entity) {
                        physic::BoundingVolume* bv2 = getComponent<ColliderComponent>(entity2)->boundingVolume;
                        physic::CollisionResultSet::Info info;
                        if (bv1 != nullptr && bv2 != nullptr) {
                            if (bv1->intersects(*bv2, info)) {
                                /*info.entity = entity2;
                                physic::CollisionResultSet::pushCollisionInfo(info);*/
                                return true;
                            }
                        }
                    }
                }
                if (!cells[i]->isPassable())
                        return true;
            }
            return false;
        }
            vector<EntityId> Grid::getEntities () {
                vector<EntityId> allEntities;
                for (unsigned int i = 0; i < cells.size(); i++) {
                    Cell *cell = cells[i];
                    if (cell != nullptr) {
                         for (unsigned int n = 0; n < cell->getNbEntitiesInside(); n++) {
                            bool contains = false;
                            for (unsigned int j = 0; j < allEntities.size(); j++) {
                                if (allEntities[j] == cell->getEntityInside(n))
                                    contains = true;
                            }
                            if (!contains) {
                                allEntities.push_back(cell->getEntityInside(n));
                            }
                        }
                    }
                }
                return allEntities;
            }

            math::Vec3f Grid::getMins () {
                return math::Vec3f(minX, minY, minZ);
            }

            Cell* Grid::getGridCellAt (math::Vec3f point) {
                math::Vec3f coordsCaseP = getCoordinatesAt(point);
                unsigned int indice = (math::Math::abs(minX) + coordsCaseP.x) + (math::Math::abs(minY) + coordsCaseP.y) * nbCellsPerRow + (math::Math::abs(minZ) + coordsCaseP.z) * nbCellsPerCol;
                //std::cout<<"get cell map at "<<point<<coordsCaseP<<minX<<std::endl<<"miny : "<<minY<<std::endl<<"minz : "<<minZ<<std::endl<<"nb Cells per row : "<<nbCellsPerRow<<std::endl<<"nb Cells per col : "<<nbCellsPerCol<<std::endl<<indice<<std::endl;
                if (indice >= 0 && indice < cells.size()) {
                    return cells[indice];
                }
                return nullptr;
            }

            math::Vec3f Grid::getCoordinatesAt(math::Vec3f &point) {
                //std::cout<<"get coordinates at, point : "<<point<<std::endl;
                math::Vec3f p = bm.unchangeOfBase(point);
                //std::cout<<"p : "<<p<<std::endl;
                math::Vec3f f;
                if (cellWidth > 0)
                    f.x = (int) p.x / cellWidth;
                else
                    f.x = 0;
                if (cellHeight > 0)
                    f.y = (int) p.y / cellHeight;
                else
                    f.y = 0;
                if (cellDepth > 0)
                    f.z = (int) p.z / cellDepth;
                else
                    f.z = 0;
                if (p.x <= 0 && cellWidth > 0)
                    f.x--;
                if (p.y <= 0 && cellHeight > 0)
                    f.y--;
                if (p.z <= 0 && cellDepth > 0)
                    f.z--;
                //std::cout<<"coordinates at : "<<point<<f<<std::endl;
                return f;
            }

            std::vector<Cell*> Grid::getCells () {
                return cells;
            }
            void Grid::checkExts () {
                //std::cout<<"mins : "<<minX<<","<<minY<<","<<minZ<<std::endl<<"maxs : "<<maxX<<","<<maxY<<","<<maxZ<<std::endl;
                minX = minY = minZ = std::numeric_limits<int>::max();
                maxX = maxY = maxZ = std::numeric_limits<int>::min();
                unsigned int nbCells=0;
                for (unsigned int i = 0; i < cells.size(); i++) {
                    if (cells[i] != nullptr) {
                        math::Vec3f point = cells[i]->getCellVolume().getCenter();
                        math::Vec3f coordsCaseP = getCoordinatesAt(point);
                        //std::cout<<"coordsCaseP : "<<coordsCaseP<<std::endl;
                        minX = (coordsCaseP.x < minX) ? coordsCaseP.x : minX;
                        minY = (coordsCaseP.y < minY) ? coordsCaseP.y : minY;
                        minZ = (coordsCaseP.z < minZ) ? coordsCaseP.z : minZ;
                        maxX = (coordsCaseP.x > maxX) ? coordsCaseP.x : maxX;
                        maxY = (coordsCaseP.y > maxY) ? coordsCaseP.y : maxY;
                        maxZ = (coordsCaseP.z > maxZ) ? coordsCaseP.z : maxZ;
                        nbCells++;
                    }
                }
                if (nbCells == 0) {
                    minX = minY = minZ = maxX = maxY = maxZ = 0;
                }
                //std::cout<<"mins : "<<minX<<","<<minY<<","<<minZ<<std::endl<<"maxs : "<<maxX<<","<<maxY<<","<<maxZ<<std::endl;
                //system("PAUSE");
            }

            math::Vec3f Grid::getSize() {
                return math::Vec3f (maxX - minX, maxY - minY, maxZ - minZ);
            }

            Cell* Grid::getGridCellAtFromCoords(math::Vec3f coords) {
                int indice = (math::Math::abs(minX) + coords.x) + (math::Math::abs(minY) + coords.y) * nbCellsPerRow + (math::Math::abs(minZ) + coords.z) * nbCellsPerCol;
                if (indice >= 0 && indice < static_cast<int>(cells.size()))
                    return cells[indice];
                return nullptr;
            }

            Grid::~Grid () {
                for (unsigned int i = 0; i < cells.size(); i++) {
                     if (cells[i] != nullptr)
                        delete cells[i];
                }
                cells.clear();
            }
        }
    }
}
