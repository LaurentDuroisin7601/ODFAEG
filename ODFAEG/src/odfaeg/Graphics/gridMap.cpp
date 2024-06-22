#include "../../../include/odfaeg/Graphics/gridMap.h"
namespace odfaeg {
    namespace graphic {
        using namespace std;
        using namespace sf;


        //Ajoute une tile.

        GridMap::GridMap (int cellWidth, int cellHeight, int cellDepth) {
            nbCasesPerRow = 0;
            nbCasesPerCol = 0;
            minX = minY = minZ = std::numeric_limits<int>::max();
            maxX = maxY = maxZ = std::numeric_limits<int>::min();
            this->cellWidth = cellWidth;
            this->cellHeight = cellHeight;
            this->cellDepth = cellDepth;
            offsetX = (cellWidth > 0) ? cellWidth * 0.5f : 1;
            offsetY = (cellHeight > 0) ? cellHeight * 0.5f : 1;
            offsetZ = (cellDepth > 0) ? cellDepth * 0.5f : 1;

        }

        int GridMap::getCellWidth() {
            return cellWidth;
        }

        int GridMap::getCellHeight() {
            return cellHeight;
        }
        int GridMap::getCellDepth() {
            return cellDepth;
        }
        int GridMap::getNbCasesPerRow () {
            return nbCasesPerRow;
        }
        int GridMap::getNbCasesPerCol () {
            return nbCasesPerCol;
        }
        void GridMap::setBaseChangementMatrix(BaseChangementMatrix bm) {
            this->bm = bm;
        }

        BaseChangementMatrix GridMap::getBaseChangementMatrix() {
            return bm;
        }

        bool GridMap::addEntity (Entity *entity) {
            //std::cout<<"add global bounds : "<<entity->getGlobalBounds().getPosition()<<entity->getGlobalBounds().getWidth()<<","<<entity->getGlobalBounds().getHeight()<<","<<entity->getGlobalBounds().getDepth()<<std::endl;
            int x = entity->getGlobalBounds().getPosition().x;
            int y = entity->getGlobalBounds().getPosition().y;
            int z = entity->getGlobalBounds().getPosition().z;
            int endX = (x + entity->getGlobalBounds().getWidth());
            int endY = (y + entity->getGlobalBounds().getHeight());
            int endZ = (z + entity->getGlobalBounds().getDepth());
            bool added = false;
            /*std::array<math::Vec2f, 4> pos;
            pos[0] = math::Vec2f(x, y);
            pos[1] = math::Vec2f(x, y + endY);
            pos[2] = math::Vec2f(x + endX, y + endY);
            pos[3] = math::Vec2f(x + endX, y);

            for (unsigned int i = 0; i < pos.size(); i++) {
                if (!(containsEntity(entity, pos[i]))) {
                    CellMap *cm = getGridCellAt(pos[i]);
                    if (cm == nullptr) {
                        createCellMap(pos[i]);
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
                        //std::cout<<"pos : "<<pos<<std::endl;


                        if (!(containsEntity(entity, pos))) {
                            //std::cout<<"get cell map"<<std::endl;
                            CellMap *cm = getGridCellAt(pos);
                            //std::cout<<"create cell map"<<std::endl;
                            if (cm == nullptr) {
                                //std::cout<<"create cell map"<<std::endl;
                                createCellMap(pos);
                                cm = getGridCellAt(pos);
                            }
                            added = true;

                            cm->addEntity(entity);
                            /*if (entity->getType() == "E_BIGTILE")
                              std::cout<<cm->getCoords()<<std::endl;*/
                            //std::cout<<"entity added"<<std::endl;
                            /*if (entity->getRootType() == "E_WALL") {
                                int indice = (math::Math::abs(minX) + cm->getCoords().x)
                                + (math::Math::abs(minY) + cm->getCoords().y) * nbCasesPerRow + (math::Math::abs(minZ) + cm->getCoords().z) * nbCasesPerCol;
                                std::cout<<"add wall at : "<<pos<<cm->getCoords()<<minX<<std::endl<<"miny : "<<minY<<std::endl<<"minz : "<<minZ<<std::endl<<"nb cases per row : "<<nbCasesPerRow<<std::endl<<"nb cases per col : "<<nbCasesPerCol<<std::endl<<"index : "<<indice<<std::endl;
                            }*/
                            /*if (i == x && j == y && k == z && entity->getType() == "E_TILE") {*/

                                /*int indice = (math::Math::abs(minX) + cm->getCoords().x)
                                                    + (math::Math::abs(minY) + cm->getCoords().y) * nbCasesPerRow + (math::Math::abs(minZ) + cm->getCoords().z) * nbCasesPerCol;
                                std::cout<<"add entity mins : "<<pos<<std::endl<<minX<<","<<minY<<","<<minZ<<std::endl<<"maxs : "<<maxX<<","<<maxY<<","<<maxZ<<std::endl<<"nb cases : "<<nbCasesPerRow<<","<<nbCasesPerCol<<std::endl<<"coords : "<<cm->getCoords()<<std::endl;*/



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

        bool GridMap::containsEntity(Entity *entity, math::Vec3f pos) {
            CellMap *caseMap = getGridCellAt(pos);
            if (caseMap !=nullptr) {
                 if (caseMap->containsEntity(entity)) {
                     return true;
                 }
            }
            return false;
        }

        Entity* GridMap::getEntity (int id) {
            for (unsigned int i = 0; i < casesMap.size(); i++) {
                CellMap *cm = casesMap[i];
                if (cm != nullptr) {
                    for (unsigned int j = 0; j < cm->getEntitiesInside().size(); j++) {
                        Entity *entity = cm->getEntityInside(j);
                        if (entity->getId() == id) {
                            return entity;
                        }
                        Entity* parent = entity->getParent();
                        while (parent != nullptr) {
                            if (parent->getId() == id) {
                                return parent;
                            }
                            parent = parent->getParent();
                        }
                    }
                }
            }
            return nullptr;
        }
        Entity* GridMap::getEntity (std::string name) {
            for (unsigned int i = 0; i < casesMap.size(); i++) {
                CellMap *cm = casesMap[i];
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
        }

        void GridMap::createCellMap (math::Vec3f &point) {
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
            physic::BoundingPolyhedron *bp = new physic::BoundingPolyhedron(v[0], v[1], v[2], true);
            bp->addTriangle(v[0], v[2], v[3]);
            //Face gauche.
            bp->addTriangle(v[0], v[1], v[7]);
            bp->addTriangle(v[0], v[3], v[7]);
            //Face droite.
            bp->addTriangle(v[1], v[5], v[6]);
            bp->addTriangle(v[1], v[2], v[6]);
            //Face de derrière.
            bp->addTriangle(v[4], v[5], v[6]);
            bp->addTriangle(v[4], v[7], v[6]);
            //Face du dessus.
            bp->addTriangle(v[0], v[4], v[5]);
            bp->addTriangle(v[0], v[1], v[5]);
            //Face du dessous.
            bp->addTriangle(v[3], v[7], v[6]);
            bp->addTriangle(v[3], v[2], v[6]);
            //std::cout<<"center : "<<bp->getCenter()<<std::endl;
            CellMap *cell = new CellMap(bp, coordsCaseP);
            casesMap.push_back(cell);
            checkExts();
            casesMap.pop_back();

            nbCasesPerRow = (cellWidth > 0) ? math::Math::abs(minX) + maxX + 1 : 1;
            nbCasesPerCol = (cellHeight > 0) ? math::Math::abs(minY) + maxY + 1 : 1;
            int nbCasesPerDepth = (cellDepth > 0) ? math::Math::abs(minZ) + maxZ + 1 : 1;
            //std::cout<<"nbCasesPerRow : "<<nbCasesPerRow<<std::endl<<"nbCasesPerCol : "<<nbCasesPerCol<<"nb cases per depth"<<nbCasesPerDepth<<std::endl;
            unsigned int newSize = nbCasesPerCol * nbCasesPerRow * nbCasesPerDepth;
            //std::cout<<"min z : "<<minZ<<std::endl;
            int indice = (math::Math::abs(minX) + coordsCaseP.x)
                         + (math::Math::abs(minY) + coordsCaseP.y) * nbCasesPerRow + (math::Math::abs(minZ) + coordsCaseP.z) * nbCasesPerCol;
            //std::cout<<"create cell map at indice : "<<indice<<std::endl;
            if (newSize > casesMap.size()) {
                //std::cout<<"resize vector! : "<<newSize<<std::endl;
                vector<CellMap*> tmpCasesMap = casesMap;
                casesMap.clear();
                casesMap.resize(newSize);
                std::fill(casesMap.begin(), casesMap.end(), nullptr);
                for (unsigned int i = 0; i < tmpCasesMap.size(); i++) {
                    if (tmpCasesMap[i] != nullptr) {
                        math::Vec3f coords = tmpCasesMap[i]->getCoords();
                        int newInd = (math::Math::abs(minX) + coords.x)
                                     + (math::Math::abs(minY) + coords.y) * nbCasesPerRow + (math::Math::abs(minZ) + coords.z) * nbCasesPerCol;
                        //std::cout<<"new ind  : "<<newInd<<std::endl;
                        casesMap[newInd] = tmpCasesMap[i];
                    }
                }
            } else if (newSize < casesMap.size()) {
                //std::cout<<"resize vector! : "<<newSize<<std::endl;
                vector<CellMap*> tmpCasesMap = casesMap;
                casesMap.clear();
                casesMap.resize(newSize);
                std::fill(casesMap.begin(), casesMap.end(), nullptr);
                for (unsigned int i = 0; i < tmpCasesMap.size(); i++) {
                    if (tmpCasesMap[i] != nullptr) {
                        math::Vec3f coords = tmpCasesMap[i]->getCoords();
                        int newInd = (math::Math::abs(minX) + coords.x)
                                     + (math::Math::abs(minY) + coords.y) * nbCasesPerRow + (math::Math::abs(minZ) + coords.z) * nbCasesPerCol;
                        //std::cout<<"new ind  : "<<newInd<<std::endl;
                        casesMap[newInd] = tmpCasesMap[i];
                    }
                }
            }
            //std::cout<<"ind : "<<indice<<std::endl;
            casesMap[indice] = cell;
            //system("PAUSE");
        }

        void GridMap::replaceEntity (Entity *entity) {
            removeEntity(entity);
            addEntity(entity);
        }

        //Supprime une tile dans la cellule. (Sans la supprimer de la mémoire.)
        bool GridMap::removeEntity (Entity *entity) {
            //std::cout<<"remove global bounds : "<<entity->getGlobalBounds().getPosition()<<entity->getGlobalBounds().getWidth()<<","<<entity->getGlobalBounds().getHeight()<<","<<entity->getGlobalBounds().getDepth()<<std::endl;
            int x = entity->getGlobalBounds().getPosition().x;
            int y = entity->getGlobalBounds().getPosition().y;
            int z = entity->getGlobalBounds().getPosition().z;
            int endX = (x + entity->getGlobalBounds().getWidth());
            int endY = (y + entity->getGlobalBounds().getHeight());
            int endZ = (z + entity->getGlobalBounds().getDepth());
            bool removed = false;
            for (int i = x; i <= endX; i+= offsetX) {
                for (int j = y; j <= endY; j+= offsetY) {
                    for (int k = z; k <= endZ; k+= offsetZ) {
                        math::Vec3f pos (i, j, k);
                        CellMap *cm = getGridCellAt(pos);
                        /*math::Vec3f coords = getCoordinatesAt(pos);
                        int indice = (math::Math::abs(minX) + coords.x)
                                    + (math::Math::abs(minY) + coords.y) * nbCasesPerRow + (math::Math::abs(minZ) + coords.z) * nbCasesPerCol;
                        std::cout<<"remove entity indice : "<<indice<<std::endl<<"mins : "<<std::endl<<minX<<","<<minY<<","<<minZ<<std::endl<<"maxs : "<<maxX<<","<<maxY<<","<<maxZ<<std::endl<<"nb cases : "<<nbCasesPerRow<<","<<nbCasesPerCol<<std::endl<<"coords : "<<coords<<"size : "<<casesMap.size()<<std::endl;*/
                        if (cm != nullptr) {
                          /*if (i == x && j == y && k == z && entity->getType() == "E_TILE") {
                              int indice = (math::Math::abs(minX) + cm->getCoords().x)
                                    + (math::Math::abs(minY) + cm->getCoords().y) * nbCasesPerRow + (math::Math::abs(minZ) + cm->getCoords().z) * nbCasesPerCol;
                          }*/
                          if(cm->removeEntity(entity)) {
                            removed = true;
                          }
                          if (!cm->isEntityInside())
                                removeCellMap(cm);
                        }
                    }
                }
            }
            return removed;
        }

        bool GridMap::deleteEntity (Entity *entity) {
            int x = entity->getGlobalBounds().getPosition().x;
            int y = entity->getGlobalBounds().getPosition().y;
            int z = entity->getGlobalBounds().getPosition().z;
            int endX = (x + entity->getGlobalBounds().getWidth());
            int endY = (y + entity->getGlobalBounds().getHeight());
            int endZ = (z + entity->getGlobalBounds().getDepth());

            bool removed = false;
            for (int i = x; i <= endX; i+= offsetX) {
                for (int j = y; j <= endY; j+= offsetY) {
                    for (int k = z; k <= endZ; k+= offsetZ) {
                        math::Vec3f pos (i, j, k);
                        /*if (entity->getType() == "E_BIGTILE")
                              std::cout<<"remove entity at : "<<pos<<std::endl;*/
                        CellMap *cm = getGridCellAt(pos);
                        if (cm != nullptr) {

                          if(!removed && cm->deleteEntity(entity))
                            removed = true;
                          else
                            cm->removeEntity(entity);
                          if (!cm->isEntityInside()) {
                                removeCellMap(cm);
                          }
                        }
                    }
                }
            }
            return removed;
        }

        bool GridMap::deleteEntity(int id) {

            vector<Entity*>::iterator it;
            vector<Entity*> entities = getEntities();
            for (it = entities.begin(); it != entities.end();) {

                if ((*it)->getId() == id) {
                    if (!deleteEntity(*it))
                        return false;
                    it = entities.erase(it);
                    return true;
                } else
                    it++;
            }
            return false;
        }

        void GridMap::removeCellMap (CellMap *cell) {

            for (unsigned int i = 0; i < casesMap.size(); i++) {
                if (casesMap[i] != nullptr && casesMap[i]==cell) {
                    //std::cout<<"delete cell : "<<casesMap[i]->getCoords()<<std::endl;
                    delete casesMap[i];
                    casesMap[i] = nullptr;
                }
            }
            //Supprime les cases vides à la fin du vecteur.
            //On recherche les coordonnées de la case la plus grande.
            checkExts();
            //On cherche si il faut réduire la taille du vecteur. (En partant du début.)
            nbCasesPerRow = (cellWidth > 0) ? math::Math::abs(minX) + maxX + 1 : 1;
            nbCasesPerCol = (cellHeight > 0) ? math::Math::abs(minY) + maxY + 1 : 1;
            int nbCasesPerDepth = (cellDepth > 0) ? math::Math::abs(minZ) + maxZ + 1 : 1;
            unsigned int newSize = nbCasesPerCol * nbCasesPerRow * nbCasesPerDepth;

            if (newSize < casesMap.size()) {
                //std::cout<<"new size : "<<newSize<<std::endl;
                vector<CellMap*> tmpCasesMap = casesMap;
                casesMap.clear();
                casesMap.resize(newSize);
                std::fill(casesMap.begin(), casesMap.end(), nullptr);
                for (unsigned int i = 0; i < tmpCasesMap.size(); i++) {
                    if (tmpCasesMap[i] != nullptr) {
                        math::Vec3f coords = tmpCasesMap[i]->getCoords();
                        int newInd = math::Math::abs(minX) + coords.x + (math::Math::abs(minY) + coords.y) * nbCasesPerRow + (math::Math::abs(minZ) + coords.z) * nbCasesPerCol;
                        casesMap[newInd] = tmpCasesMap[i];
                    }
                }
            } else if (newSize > casesMap.size()) {
                //std::cout<<"new size : "<<newSize<<std::endl;
                vector<CellMap*> tmpCasesMap = casesMap;
                casesMap.clear();
                casesMap.resize(newSize);
                std::fill(casesMap.begin(), casesMap.end(), nullptr);
                for (unsigned int i = 0; i < tmpCasesMap.size(); i++) {
                    if (tmpCasesMap[i] != nullptr) {
                        math::Vec3f coords = tmpCasesMap[i]->getCoords();
                        int newInd = math::Math::abs(minX) + coords.x + (math::Math::abs(minY) + coords.y) * nbCasesPerRow + (math::Math::abs(minZ) + coords.z) * nbCasesPerCol;
                        casesMap[newInd] = tmpCasesMap[i];
                    }
                }
            }
        }

        vector<CellMap*> GridMap::getCasesInBox (physic::BoundingBox bx) {

            vector<CellMap*> cells;
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
                        CellMap *cell = getGridCellAt(p);
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

        vector<Entity*> GridMap::getEntitiesInBox(physic::BoundingBox box) {
            vector<Entity*> entities;
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
                        CellMap* cell = getGridCellAt(point);
                        if (cell != nullptr) {
                            for (unsigned int n = 0; n < cell->getEntitiesInside().size(); n++) {
                               Entity* entity = cell->getEntityInside(n);
                               physic::BoundingBox bx2 = entity->getGlobalBounds();
                               bool contains = false;
                               for (unsigned int k = 0; k < entities.size() && !contains; k++) {
                                    if (entities[k] == entity)
                                        contains = true;
                               }
                               if (!contains/* && bx.intersects(bx2) || bx.isInside(bx2) || bx2.isInside(bx)*/) {

                                    entities.push_back(entity);
                               }
                            }
                        }
                    }
                }
            }
            return entities;
        }

        vector<Entity*> GridMap::getEntities () {
            vector<Entity*> allEntities;
            for (unsigned int i = 0; i < casesMap.size(); i++) {
                CellMap *cell = casesMap[i];
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

        math::Vec3f GridMap::getMins () {
            return math::Vec3f(minX, minY, minZ);
        }

        CellMap* GridMap::getGridCellAt (math::Vec3f point) {
            math::Vec3f coordsCaseP = getCoordinatesAt(point);
            unsigned int indice = (math::Math::abs(minX) + coordsCaseP.x) + (math::Math::abs(minY) + coordsCaseP.y) * nbCasesPerRow + (math::Math::abs(minZ) + coordsCaseP.z) * nbCasesPerCol;
            /*if (point.x == 50 && point.y == 25 && point.z == 75)
                std::cout<<"get cell map at "<<point<<"coordCaseP : "<<coordsCaseP<<"minX : "<<minX<<std::endl<<"miny : "<<minY<<std::endl<<"minz : "<<minZ<<std::endl<<"nb cases per row : "<<nbCasesPerRow<<std::endl<<"nb cases per col : "<<nbCasesPerCol<<std::endl<<"indice : "<<indice<<std::endl;*/
            if (indice >= 0 && indice < casesMap.size()) {
                return casesMap[indice];
            }
            return nullptr;
        }

        math::Vec3f GridMap::getCoordinatesAt(math::Vec3f &point) {
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

        std::vector<CellMap*> GridMap::getCasesMap () {
            return casesMap;
        }
        bool GridMap::collideWithEntity(Entity* entity, math::Vec3f position) {
            CellMap* cell = getGridCellAt(position);
            if (cell != nullptr) {
                if (!cell->isPassable())
                    return true;
                /*std::vector<CellMap*> neightbours = getNeightbours(entity,cell,true);
                for (unsigned int i = 0; i < neightbours.size(); i++) {
                    if (!neightbours[i]->isPassable()) {
                        return true;
                    }
                }*/
                if (entity->getRootEntity()->getCollisionVolume() != nullptr) {
                    math::Vec3f t = position - entity->getRootEntity()->getCollisionVolume()->getCenter();
                    physic::BoundingVolume* cv = entity->getRootEntity()->getCollisionVolume()->clone().release();
                    cv->move(t);
                    for (unsigned int k = 0; k < cell->getEntitiesInside().size(); k++)  {
                        if (cell->getEntitiesInside()[k]->getRootEntity()->getCollisionVolume() != nullptr && cell->getEntitiesInside()[k]->getRootEntity() != entity) {
                            physic::CollisionResultSet::Info info;
                            if (cv->intersects(*cell->getEntitiesInside()[k]->getRootEntity()->getCollisionVolume(), info)) {
                                info.entity = cell->getEntitiesInside()[k]->getRootEntity();
                                info.center = cv->getCenter();
                                physic::CollisionResultSet::pushCollisionInfo(info);
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
        bool GridMap::collideWithEntity(Entity* entity) {
            std::unique_ptr<physic::BoundingVolume> bv1;
            physic::BoundingBox bx(entity->getGlobalBounds().getPosition().x, entity->getGlobalBounds().getPosition().y,entity->getGlobalBounds().getPosition().z,entity->getGlobalBounds().getWidth(),entity->getGlobalBounds().getHeight(),entity->getGlobalBounds().getDepth());
            if (entity->getCollisionVolume() != nullptr) {
                bv1 = entity->getCollisionVolume()->clone();
            }
            std::vector<CellMap*> cells = getCasesInBox(bx);
            for (unsigned int i = 0; i < cells.size(); i++) {
                for (unsigned int j = 0; j < cells[i]->getEntitiesInside().size(); j++) {
                    Entity* entity2 = cells[i]->getEntitiesInside()[j]->getRootEntity();
                    if (entity2 != entity) {
                        physic::BoundingVolume* bv2 = entity2->getCollisionVolume();
                        physic::CollisionResultSet::Info info;
                        if (bv1 != nullptr && bv2 != nullptr) {
                            if (bv1->intersects(*bv2, info)) {
                                info.entity = entity2;
                                physic::CollisionResultSet::pushCollisionInfo(info);
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

        void GridMap::checkExts () {
            //std::cout<<"mins : "<<minX<<","<<minY<<","<<minZ<<std::endl<<"maxs : "<<maxX<<","<<maxY<<","<<maxZ<<std::endl;
            minX = minY = minZ = std::numeric_limits<int>::max();
            maxX = maxY = maxZ = std::numeric_limits<int>::min();
            unsigned int nbCases=0;
            for (unsigned int i = 0; i < casesMap.size(); i++) {
                if (casesMap[i] != nullptr) {
                    math::Vec3f point = casesMap[i]->getCellVolume()->getCenter();
                    math::Vec3f coordsCaseP = getCoordinatesAt(point);
                    //std::cout<<"coordsCaseP : "<<coordsCaseP<<std::endl;
                    minX = (coordsCaseP.x < minX) ? coordsCaseP.x : minX;
                    minY = (coordsCaseP.y < minY) ? coordsCaseP.y : minY;
                    minZ = (coordsCaseP.z < minZ) ? coordsCaseP.z : minZ;
                    maxX = (coordsCaseP.x > maxX) ? coordsCaseP.x : maxX;
                    maxY = (coordsCaseP.y > maxY) ? coordsCaseP.y : maxY;
                    maxZ = (coordsCaseP.z > maxZ) ? coordsCaseP.z : maxZ;
                    nbCases++;
                }
            }
            if (nbCases == 0) {
                minX = minY = minZ = maxX = maxY = maxZ = 0;
            }
            //std::cout<<"mins : "<<minX<<","<<minY<<","<<minZ<<std::endl<<"maxs : "<<maxX<<","<<maxY<<","<<maxZ<<std::endl;
            //system("PAUSE");
        }

        math::Vec3f GridMap::getSize() {
            return math::Vec3f (maxX - minX, maxY - minY, maxZ - minZ);
        }
        vector<CellMap*> GridMap::getNeightbours(Entity* entity, CellMap *cell, bool getCellOnPassable) {
            math::Vec3f coords = cell->getCoords();
            vector<CellMap*> neightbours;
            for (int i = coords.x - 1; i <= coords.x + 1; i++) {
                for (int j = coords.y - 1; j <= coords.y + 1; j++) {
                    for (int k = coords.z - 1; k <= coords.z + 1; k++) {
                        if (!(i == coords.x && j == coords.y && k == coords.z)) {
                            math::Vec2f neightbourCoords(i, j);
                            CellMap *neightbour = getGridCellAtFromCoords(neightbourCoords);
                            if (neightbour != nullptr) {
                                if (getCellOnPassable)
                                    neightbours.push_back(neightbour);
                                else {
                                    if (entity->getCollisionVolume() != nullptr) {
                                        bool collide = false;
                                        math::Vec3f t = neightbour->getCenter() - entity->getCollisionVolume()->getCenter();
                                        std::unique_ptr<physic::BoundingVolume> cv = entity->getCollisionVolume()->clone();
                                        cv->move(t);
                                        for (unsigned int k = 0; k < neightbour->getEntitiesInside().size() && !collide; k++) {
                                            if (neightbour->getEntitiesInside()[k]->getCollisionVolume() != nullptr && neightbour->getEntitiesInside()[k] != entity) {
                                                physic::CollisionResultSet::Info info;
                                                if (cv->intersects(*neightbour->getEntitiesInside()[k]->getCollisionVolume(), info)) {
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

        vector<math::Vec3f> GridMap::getPath (Entity* entity, math::Vec3f finalPos) {
            math::Vec3f startPos = math::Vec3f(entity->getCenter().x, entity->getCenter().y, entity->getCenter().z);
            vector<math::Vec3f> positions;
            if (getGridCellAt(finalPos) != nullptr) {
                unsigned int cpt = 0;
                positions.push_back(startPos);
                math::Vec3f currentPos = getGridCellAt(startPos)->getCenter();
                if (currentPos != startPos)
                    positions.push_back(currentPos);
                vector<CellMap*> children;
                while (!getGridCellAt(finalPos)->isPassable() && cpt < 1000) {
                    CellMap *parent = getGridCellAt(finalPos);

                    vector<CellMap*> children = getNeightbours(entity, parent, true);

                    int distMin = children[0]->getCenter().computeDist(getGridCellAt(startPos)->getCenter());
                    int indMin = 0;
                    for (unsigned int i = 1; i < children.size(); i++) {
                        std::cout<<children[i]->getCenter()<<std::endl;
                        int dist = children[i]->getCenter().computeDist(getGridCellAt(startPos)->getCenter());
                        if (dist < distMin) {
                            distMin = dist;
                            indMin = i;
                        }
                    }
                    finalPos = children[indMin]->getCenter();
                    cpt++;
                }
                //Tant qu'on est pas arrivé sur la case finale.
                while (getGridCellAt(currentPos) != getGridCellAt(finalPos) && cpt < 1000) {
                    //On recherche les cases fils. (ou voisines.)

                    CellMap *parent = getGridCellAt(currentPos);
                    parent->setTraveled(true);
                    vector<CellMap*> children = getNeightbours(entity, parent, false);
                    //std::cout<<"size : "<<children.size()<<std::endl;
                    unsigned int j = 0;
                    while (j < children.size() && children[j]->isTraveled())
                        j++;
                    if (j == children.size())
                        j--;
                    int distMin = children[j]->getCenter().computeDist(getGridCellAt(finalPos)->getCenter());
                    int indMin = j;
                    for (unsigned int i = j; i < children.size(); i++) {
                        if (!children[i]->isTraveled()) {
                            int dist = children[i]->getCenter().computeDist(getGridCellAt(finalPos)->getCenter());

                            if (dist < distMin) {

                                distMin = dist;
                                indMin = i;
                            }
                        }
                    }
                    currentPos = children[indMin]->getCenter();
                    if (positions.size() > 1) {
                        math::Vec3f v1 = positions[positions.size()-1] - positions[positions.size() - 2];
                        math::Vec3f v2 = currentPos - positions[positions.size()-1];
                        if (math::Math::abs(math::Math::toDegrees(v1.getAngleBetween(v2, v1.cross(v2)))) != 180)
                            positions.push_back(currentPos);
                    } else {
                         positions.push_back(currentPos);
                    }
                    cpt++;
                }
                if (finalPos != positions.back())
                    positions.push_back(finalPos);
                for (unsigned int i = 0; i < positions.size(); i++) {
                     getGridCellAt(positions[i])->setTraveled(false);
                }
                if (cpt == 1000)
                    positions.clear();
            }
            return positions;
        }

        CellMap* GridMap::getGridCellAtFromCoords(math::Vec3f coords) {
            int indice = (math::Math::abs(minX) + coords.x) + (math::Math::abs(minY) + coords.y) * nbCasesPerRow + (math::Math::abs(minZ) + coords.z) * nbCasesPerCol;
            if (indice >= 0 && indice < static_cast<int>(casesMap.size()))
                return casesMap[indice];
            return nullptr;
        }

        GridMap::~GridMap () {
            /*vector<Entity*> entities = getEntities();
            for (unsigned int i = 0; i < entities.size(); i++) {
                delete entities[i];
            }*/
            for (unsigned int i = 0; i < casesMap.size(); i++) {
                 if (casesMap[i] != nullptr)
                    delete casesMap[i];
            }
            casesMap.clear();
        }
    }
}



