namespace odfaeg {
    namespace entity {
        using namespace std;
        template<typename Object>
        GridMap<Object>::GridMap (float cellWidth, float cellHeight, float cellDepth) {
            nbCasesPerRow = 0;
            nbCasesPerCol = 0;
            minX = minY = minZ = maxX = maxY = maxZ = 0;
            this->cellWidth = cellWidth;
            this->cellHeight = cellHeight;
            this->cellDepth = cellDepth;
        }
        template<typename Object>
        int GridMap<Object>::getCellWidth() {
            return cellWidth;
        }
        template<typename Object>
        int GridMap<Object>::getCellHeight() {
            return cellHeight;
        }
        template<typename Object>
        int GridMap<Object>::getCellDepth() {
            return cellDepth;
        }
        template<typename Object>
        int GridMap<Object>::getNbCasesPerRow () {
            return nbCasesPerRow;
        }
        template<typename Object>
        int GridMap<Object>::getNbCasesPerCol () {
            return nbCasesPerCol;
        }
        /*template<typneme Object>
        void GridMap<Object>::setBaseChangementMatrix(BaseChangementMatrix bm) {
            this->bm = bm;
        }
        template<typname Object>
        BaseChangementMatrix GridMap<Object>::getBaseChangementMatrix() {
            return bm;
        }*/
        template<typename Object>
        bool GridMap<Object>::addEntity (Object entity, physic::BoundingBox objectVolume) {

            /*if (entity->getType() == "E_ANIMATION_FRAME")
                //////std::cout<<"add global bounds : "<<entity->getGlobalBounds().getPosition()<<entity->getGlobalBounds().getWidth()<<","<<entity->getGlobalBounds().getHeight()<<","<<entity->getGlobalBounds().getDepth()<<std::endl;*/
            float x = objectVolume.getPosition().x();
            float y = objectVolume.getPosition().y();
            float z = objectVolume.getPosition().z();
            float endX = (x + objectVolume.getWidth());
            float endY = (y + objectVolume.getHeight());
            float endZ = (z + objectVolume.getDepth());
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
            //////////std::cout<<"offsets : "<<offsetX<<","<<offsetY<<","<<offsetZ<<std::endl<<"start : "<<x<<","<<y<<","<<z<<std::endl<<"ends : "<<endX<<","<<endY<<","<<endZ<<std::endl;
            //////////std::cout<<"add entity : "<<entity->getType()<<std::endl<<x<<","<<y<<","<<z<<","<<endX<<","<<endY<<","<<endZ<<std::endl;
            for (float i = x; i <= endX; i+= cellWidth) {
                for (float j = y; j <= endY; j+= cellHeight)  {
                    for (float k = z; k <= endZ; k+= cellDepth) {

                        math::Vec3f pos (i, j, k);
                        //std::cout<<"pos : "<<pos<<std::endl;

                        ////////std::cout<<"contains entity"<<std::endl;
                        if (!(containsEntity(entity, pos))) {
                            //std::cout<<"contains ok"<<std::endl;

                            GridCell<Object> *cm = getGridCellAt(pos);
                            //std::cout<<"cm ok"<<std::endl;
                            if (cm == nullptr) {
                                //std::cout<<"create cell map"<<std::endl;
                                createCellMap(pos);
                                /*std::cout<<"cell map created"<<std::endl;
                                std::cout<<"get cell map"<<std::endl;*/
                                cm = getGridCellAt(pos);
                                
                            }
                            added = true;
                            //std::cout<<"add entity at : "<<objectVolume.getPosition()<<","<<std::endl;
                            cm->addEntity(entity, objectVolume);
                            //std::cout<<"cell map : "<<cm<<std::endl;
                            //std::cout<<"entity added to cell"<<std::endl;

                            /*if (entity->getType() == "E_BIGTILE")
                              ////////std::cout<<cm->getCoords()<<std::endl;*/
                            //////////std::cout<<"entity added"<<std::endl;
                            /*if (entity->getRootType() == "E_WALL") {
                                int indice = (math::Math::abs(minX) + cm->getCoords().x)
                                + (math::Math::abs(minY) + cm->getCoords().y) * nbCasesPerRow + (math::Math::abs(minZ) + cm->getCoords().z) * nbCasesPerCol;
                                ////////std::cout<<"add wall at : "<<pos<<cm->getCoords()<<minX<<std::endl<<"miny : "<<minY<<std::endl<<"minz : "<<minZ<<std::endl<<"nb cases per row : "<<nbCasesPerRow<<std::endl<<"nb cases per col : "<<nbCasesPerCol<<std::endl<<"index : "<<indice<<std::endl;
                            }*/
                            /*if (i == x && j == y && k == z && entity->getType() == "E_TILE") {*/

                                /*int indice = (math::Math::abs(minX) + cm->getCoords().x)
                                                    + (math::Math::abs(minY) + cm->getCoords().y) * nbCasesPerRow + (math::Math::abs(minZ) + cm->getCoords().z) * nbCasesPerCol;
                                ////////std::cout<<"add entity mins : "<<pos<<std::endl<<minX<<","<<minY<<","<<minZ<<std::endl<<"maxs : "<<maxX<<","<<maxY<<","<<maxZ<<std::endl<<"nb cases : "<<nbCasesPerRow<<","<<nbCasesPerCol<<std::endl<<"coords : "<<cm->getCoords()<<std::endl;*/



                                //system("PAUSE");
                            //}

                        }
                        ////////std::cout<<"contains entity ended"<<std::endl;
                    }
                    //////////std::cout<<"leave k"<<std::endl;
                }
                //////////std::cout<<"leave j"<<std::endl;
            }
            ////////std::cout<<"entity added"<<std::endl;
            return added;
        }
        template<typename Object>
        bool GridMap<Object>::containsEntity(Object entity, math::Vec3f pos) {
            GridCell<Object> *caseMap = getGridCellAt(pos);
            if (caseMap !=nullptr) {
                 if (caseMap->containsEntity(entity)) {
                     return true;
                 }
            }
            return false;
        }
        template<typename Object>
        Object* GridMap<Object>::getEntity (int id) {
            for (unsigned int i = 0; i < casesMap.size(); i++) {
                GridCell<Object> cm = casesMap[i];
                if (!cm.empty()) {
                    for (unsigned int j = 0; j < cm->getEntitiesInside().size(); j++) {
                        Object entity = cm->getEntityInside(j);
                        if (entity->getId() == id) {
                            return entity;
                        }
                    }
                }
            }
            return nullptr;
        }
        template<typename Object>
        Object* GridMap<Object>::getEntity (std::string name) {
            for (unsigned int i = 0; i < casesMap.size(); i++) {
                GridCell<Object> cm = casesMap[i];
                if (!cm.empty()) {
                    for (unsigned int j = 0; j < cm->getEntitiesInside().size(); j++) {
                        Object entity = cm->getEntityInside(j);
                        if (entity->getName() == name) {
                            return entity;
                        }                        
                    }
                }
            }
            return nullptr;
        }
        template<typename Object>
        void GridMap<Object>::createCellMap (math::Vec3f &point) {
            ////////std::cout<<"point : "<<point<<std::endl;
            math::Vec3f p = getCoordinatesAt(point);
            //std::cout<<"coords caseP : "<<p<<std::endl;

            /*minX = (coordsCaseP.x < minX) ? coordsCaseP.x : minX;
            minY = (coordsCaseP.y < minY) ? coordsCaseP.y : minY;
            minZ = (coordsCaseP.z < minZ) ? coordsCaseP.z : minZ;
            maxX = (coordsCaseP.x > maxX) ? coordsCaseP.x : maxX;
            maxY = (coordsCaseP.y > maxY) ? coordsCaseP.y : maxY;
            maxZ = (coordsCaseP.z > maxZ) ? coordsCaseP.z : maxZ;*/

            //math::Vec3f p = bm.unchangeOfBase(point);
            /*if (p.x() <= 0)
                p[0]--;
            if (p.y() <= 0)
                p[1]--;
            if (p.z() <= 0)
                p[2]--;*/

            math::Vec3f v1;
            v1[0] = (cellWidth > 0) ? std::floor(point.x() / cellWidth) : 0;
            v1[1] = (cellHeight > 0) ? std::floor(point.y() / cellHeight) : 0;
            v1[2] = (cellDepth > 0) ? std::floor(point.z() / cellDepth) : 0;
            /*if (point.x() < 0)
                v1[0]--;
            if (point.y() < 0)
                v1[1]--;
            if (point.z() < 0)
                v1[2]--;*/
            v1[0] *= cellWidth;
            v1[1] *= cellHeight;
            v1[2] *= cellDepth;
            /*math::Vec3f v[8];
            v[0] = math::Vec3f (v1.x(), v1.y(), v1.z());
            v[1] = math::Vec3f (v1.x() + cellWidth, v1.y(), v1.z());
            v[2] = math::Vec3f (v1.x() + cellWidth, v1.y() + cellHeight, v1.z());
            v[3] = math::Vec3f (v1.x(), v1.y() + cellHeight, v1.z());
            v[4] = math::Vec3f (v1.x(), v1.y(), v1.z()+cellDepth);
            v[5] = math::Vec3f (v1.x() + cellWidth, v1.y(), v1.z()+cellDepth);
            v[6] = math::Vec3f (v1.x() + cellWidth, v1.y() + cellHeight, v1.z()+cellDepth);
            v[7] = math::Vec3f (v1.x(), v1.y() + cellHeight, v1.z()+cellDepth);

            for (unsigned int i = 0; i < 8; i++) {
                v[i] = bm.changeOfBase(v[i]);               
            }*/

            //Face de devant.
            physic::BoundingBox volume (v1[0], v1[1], v1[2], cellWidth, cellHeight, cellDepth);
            /*bp->addTriangle(v[0], v[2], v[3]);
            //Face gauche.
            bp->addTriangle(v[0], v[1], v[7]);
            bp->addTriangle(v[0], v[3], v[7]);
            //Face droite.
            bp->addTriangle(v[1], v[5], v[6]);
            bp->addTriangle(v[1], v[2], v[6]);
            //Face de derri�re.
            bp->addTriangle(v[4], v[5], v[6]);
            bp->addTriangle(v[4], v[7], v[6]);
            //Face du dessus.
            bp->addTriangle(v[0], v[4], v[5]);
            bp->addTriangle(v[0], v[1], v[5]);
            //Face du dessous.
            bp->addTriangle(v[3], v[7], v[6]);
            bp->addTriangle(v[3], v[2], v[6]);*/
            //////////std::cout<<"center : "<<bp->getCenter()<<std::endl;
            GridCell<Object> cell(volume, p);
            casesMap.push_back(cell);
            checkExts();
            casesMap.pop_back();

            nbCasesPerRow = (cellWidth > 0) ? math::Math::abs(minX) + maxX + 1 : 1;
            nbCasesPerCol = (cellHeight > 0) ? math::Math::abs(minY) + maxY + 1 : 1;
            int nbCasesPerDepth = (cellDepth > 0) ? math::Math::abs(minZ) + maxZ + 1 : 1;
            //////////std::cout<<"nbCasesPerRow : "<<nbCasesPerRow<<std::endl<<"nbCasesPerCol : "<<nbCasesPerCol<<"nb cases per depth"<<nbCasesPerDepth<<std::endl;
            unsigned int newSize = nbCasesPerCol * nbCasesPerRow * nbCasesPerDepth;
            //////////std::cout<<"min z : "<<minZ<<std::endl;
            int indice = (math::Math::abs(minX) + p.x())
                         + (math::Math::abs(minY) + p.y()) * nbCasesPerRow + (math::Math::abs(minZ) + p.z()) * nbCasesPerCol * nbCasesPerRow;
            //std::cout<<"create cell map at indice : "<<math::Math::abs(minX)<<","<<p.x()<<","<<casesMap.size()<<std::endl;
            if (newSize > casesMap.size()) {
                ////////std::cout<<"resize vector! > : "<<newSize<<std::endl;
                vector<GridCell<Object>> tmpCasesMap = casesMap;
                casesMap.clear();
                casesMap.resize(newSize);
                //std::fill(casesMap.begin(), casesMap.end(), GridCell<Object>(volume, p));
                for (unsigned int i = 0; i < tmpCasesMap.size(); i++) {
                    //if (!tmpCasesMap[i].empty()) {
                        math::Vec3f coords = tmpCasesMap[i].getCoords();
                        int newInd = (math::Math::abs(minX) + coords.x())
                                     + (math::Math::abs(minY) + coords.y()) * nbCasesPerRow + (math::Math::abs(minZ) + coords.z()) * nbCasesPerCol * nbCasesPerRow;
                        //////////std::cout<<"new ind  : "<<newInd<<std::endl;
                        casesMap[newInd] = tmpCasesMap[i];
                    //}
                }
                ////////std::cout<<"vector resized > "<<std::endl;
            } else if (newSize < casesMap.size()) {
                ////////std::cout<<"resize vector < ! : "<<newSize<<std::endl;
                vector<GridCell<Object>> tmpCasesMap = casesMap;
                casesMap.clear();
                casesMap.resize(newSize);
                //std::fill(casesMap.begin(), casesMap.end(), nullptr);
                for (unsigned int i = 0; i < tmpCasesMap.size(); i++) {
                    //if (tmpCasesMap[i] != nullptr) {
                        math::Vec3f coords = tmpCasesMap[i].getCoords();
                        int newInd = (math::Math::abs(minX) + coords.x())
                                     + (math::Math::abs(minY) + coords.y()) * nbCasesPerRow + (math::Math::abs(minZ) + coords.z()) * nbCasesPerCol * nbCasesPerRow;
                        //////////std::cout<<"new ind  : "<<newInd<<std::endl;
                        casesMap[newInd] = tmpCasesMap[i];
                    //}
                }
                ////////std::cout<<"vector resized <"<<std::endl;
            }
            //////////std::cout<<"ind : "<<indice<<std::endl;
            casesMap[indice] = cell;
            //system("PAUSE");
        }
        template<typename Object>
        void GridMap<Object>::replaceEntity (Object entity) {
            removeEntity(entity);
            addEntity(entity);
        }
        template<typename Object>
        //Supprime une tile dans la cellule. (Sans la supprimer de la m�moire.)
        bool GridMap<Object>::removeEntity (Object entity, physic::BoundingBox volume) {
            /*if (entity->getType() == "E_ANIMATION_FRAME")
                //////std::cout<<"remove global bounds : "<<entity->getGlobalBounds().getPosition()<<entity->getGlobalBounds().getWidth()<<","<<entity->getGlobalBounds().getHeight()<<","<<entity->getGlobalBounds().getDepth()<<std::endl;*/
            int x = volume.getPosition().x();
            int y = volume.getPosition().y();
            int z = volume.getPosition().z();
            int endX = (x + volume.getWidth());
            int endY = (y + volume.getHeight());
            int endZ = (z + volume.getDepth());
            bool removed = false;
            for (int i = x; i <= endX; i+= cellWidth) {
                for (int j = y; j <= endY; j+= cellHeight) {
                    for (int k = z; k <= endZ; k+= cellDepth) {
                        math::Vec3f pos (i, j, k);
                        GridCell<Object> *cm = getGridCellAt(pos);
                        /*math::Vec3f coords = getCoordinatesAt(pos);
                        int indice = (math::Math::abs(minX) + coords.x)
                                    + (math::Math::abs(minY) + coords.y) * nbCasesPerRow + (math::Math::abs(minZ) + coords.z) * nbCasesPerCol;
                        ////////std::cout<<"remove entity indice : "<<indice<<std::endl<<"mins : "<<std::endl<<minX<<","<<minY<<","<<minZ<<std::endl<<"maxs : "<<maxX<<","<<maxY<<","<<maxZ<<std::endl<<"nb cases : "<<nbCasesPerRow<<","<<nbCasesPerCol<<std::endl<<"coords : "<<coords<<"size : "<<casesMap.size()<<std::endl;*/
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
        template<typename Object>
        bool GridMap<Object>::deleteEntity (Object entity, physic::BoundingBox volume) {
            int x = volume.getPosition().x();
            int y = volume.getPosition().y();
            int z = volume.getPosition().z();
            int endX = (x + volume.getWidth());
            int endY = (y + volume.getHeight());
            int endZ = (z + volume.getDepth());

            bool removed = false;
            for (int i = x; i <= endX; i+= cellWidth) {
                for (int j = y; j <= endY; j+= cellHeight) {
                    for (int k = z; k <= endZ; k+= cellDepth) {
                        math::Vec3f pos (i, j, k);
                        /*if (entity->getType() == "E_BIGTILE")
                              ////////std::cout<<"remove entity at : "<<pos<<std::endl;*/
                        GridCell<Object> *cm = getGridCellAt(pos);
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
        template<typename Object>
        bool GridMap<Object>::deleteEntity(int id) {
            typename vector<Object>::iterator it;
            vector<Object> entities = getEntities();
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
        template<typename Object>
        void GridMap<Object>::removeCellMap (GridCell<Object> *cell) {

            for (unsigned int i = 0; i < casesMap.size(); i++) {
                if (casesMap[i] != nullptr && casesMap[i]==cell) {
                    //////////std::cout<<"delete cell : "<<casesMap[i]->getCoords()<<std::endl;
                    delete casesMap[i];
                    casesMap[i] = nullptr;
                }
            }
            //Supprime les cases vides � la fin du vecteur.
            //On recherche les coordonn�es de la case la plus grande.
            checkExts();
            //On cherche si il faut r�duire la taille du vecteur. (En partant du d�but.)
            nbCasesPerRow = (cellWidth > 0) ? math::Math::abs(minX) + maxX + 1 : 1;
            nbCasesPerCol = (cellHeight > 0) ? math::Math::abs(minY) + maxY + 1 : 1;
            int nbCasesPerDepth = (cellDepth > 0) ? math::Math::abs(minZ) + maxZ + 1 : 1;
            unsigned int newSize = nbCasesPerCol * nbCasesPerRow * nbCasesPerDepth;

            if (newSize < casesMap.size()) {
                //////////std::cout<<"new size : "<<newSize<<std::endl;
                vector<GridCell<Object>> tmpCasesMap = casesMap;
                casesMap.clear();
                casesMap.resize(newSize);
                std::fill(casesMap.begin(), casesMap.end(), nullptr);
                for (unsigned int i = 0; i < tmpCasesMap.size(); i++) {
                    //if (tmpCasesMap[i] != nullptr) {
                        math::Vec3f coords = tmpCasesMap[i]->getCoords();
                        int newInd = math::Math::abs(minX) + coords.x() + (math::Math::abs(minY) + coords.y()) * nbCasesPerRow + (math::Math::abs(minZ) + coords.z()) * nbCasesPerCol * nbCasesPerRow;
                        casesMap[newInd] = tmpCasesMap[i];
                    //}
                }
            } else if (newSize > casesMap.size()) {
                //////////std::cout<<"new size : "<<newSize<<std::endl;
                vector<GridCell<Object>*> tmpCasesMap = casesMap;
                casesMap.clear();
                casesMap.resize(newSize);
                std::fill(casesMap.begin(), casesMap.end(), nullptr);
                for (unsigned int i = 0; i < tmpCasesMap.size(); i++) {
                    if (tmpCasesMap[i] != nullptr) {
                        math::Vec3f coords = tmpCasesMap[i]->getCoords();
                        int newInd = math::Math::abs(minX) + coords.x() + (math::Math::abs(minY) + coords.y()) * nbCasesPerRow + (math::Math::abs(minZ) + coords.z()) * nbCasesPerCol * nbCasesPerRow;
                        casesMap[newInd] = tmpCasesMap[i];
                    }
                }
            }
        }
        template<typename Object>
        vector<GridCell<Object>*> GridMap<Object>::getCasesInBox (physic::BoundingBox bx) {

            vector<GridCell<Object>*> cells;
            int x = bx.getPosition().x();
            int y = bx.getPosition().y();
            int z = bx.getPosition().z();
            int endX = (x + bx.getWidth());
            int endY = (y + bx.getHeight());
            int endZ = (z + bx.getDepth());
            for (int i = x; i <= endX; i+= cellWidth) {
                for (int j = y; j <= endY; j+= cellHeight) {
                    for (int k = 0; k <= endZ; k+= cellDepth) {
                        math::Vec3f p (i, j, k);
                        GridCell<Object> *cell = getGridCellAt(p);
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
        template<typename Object>
        vector<Object> GridMap<Object>::getEntitiesInBox(physic::BoundingBox box) {
            vector<Object> entities;
            int x = box.getPosition().x();
            int y = box.getPosition().y();
            int z = box.getPosition().z();
            int endX = box.getPosition().x() + box.getWidth();
            int endY = box.getPosition().y() + box.getHeight();
            int endZ = box.getPosition().z() + box.getDepth();
            physic::BoundingBox bx (x, y, z, endX-x, endY-y, z - endZ);
            for (int i = x; i <= endX; i+=cellWidth) {
                for (int j = y; j <= endY; j+=cellHeight) {
                    for (int k = z; k <= endZ; k+= cellDepth) {
                        math::Vec3f point(i, j, k);
                        GridCell<Object>* cell = getGridCellAt(point);
                        if (cell != nullptr) {
                            for (unsigned int n = 0; n < cell->getEntitiesInside().size(); n++) {
                               physic::BoundingBox bx2;
                               Object entity = cell->getEntityInside(n, bx2);                               
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
        template<typename Object>
        vector<Object> GridMap<Object>::getEntities () {
            vector<Object> allEntities;
            for (unsigned int i = 0; i < casesMap.size(); i++) {
                GridCell<Object> cell = casesMap[i];
                //if (cell != nullptr) {
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
                //}
            }
            return allEntities;
        }
        template<typename Object>
        math::Vec3f GridMap<Object>::getMins () {
            return math::Vec3f(minX, minY, minZ);
        }
        template<typename Object>
        GridCell<Object>* GridMap<Object>::getGridCellAt (math::Vec3f point) {
            math::Vec3f coordsCaseP = getCoordinatesAt(point);
            //std::cout<<"indice : "<<coordsCaseP<<std::endl;
            unsigned int indice = (math::Math::abs(minX) + coordsCaseP.x()) + (math::Math::abs(minY) + coordsCaseP.y()) * nbCasesPerRow + (math::Math::abs(minZ) + coordsCaseP.z()) * nbCasesPerCol * nbCasesPerRow;
            //std::cout<<"indice : "<<indice<<" size : "<<casesMap.size()<<std::endl;
            //system("PAUSE");
            if (indice >= 0 && indice < casesMap.size()) {
                return &casesMap[indice];
            }
            return nullptr;
        }
        template<typename Object>
        math::Vec3f GridMap<Object>::getCoordinatesAt(math::Vec3f &p) {
            //////////std::cout<<"get coordinates at, point : "<<point<<std::endl;
            //math::Vec3f p = bm.unchangeOfBase(point);
            ////////std::cout<<"point : "<<point<<std::endl;
            math::Vec3f f;
            if (cellWidth > 0)
                f[0] = std::floor(p.x() / cellWidth);
            else
                f[0] = 0;
            if (cellHeight > 0)
                f[1] = std::floor(p.y() / cellHeight);
            else
                f[1] = 0;
            if (cellDepth > 0)
                f[2] = std::floor(p.z() / cellDepth);
            else
                f[2] = 0;
            /*if (p.x() < 0 && cellWidth > 0)
                f[0]--;
            if (p.y() < 0 && cellHeight > 0)
                f[1]--;
            if (p.z() < 0 && cellDepth > 0)
                f[2]--;*/
            //std::cout<<"coordinates at : "<<f<<std::endl;
            return f;
        }
        template<typename Object>
        std::vector<GridCell<Object>> GridMap<Object>::getCasesMap () {
            return casesMap;
        }
        template<typename Object>
        void GridMap<Object>::checkExts () {
            //////////std::cout<<"mins : "<<minX<<","<<minY<<","<<minZ<<std::endl<<"maxs : "<<maxX<<","<<maxY<<","<<maxZ<<std::endl;
            minX = minY = minZ = std::numeric_limits<int>::max();
            maxX = maxY = maxZ = std::numeric_limits<int>::min();
            unsigned int nbCases=0;
            for (unsigned int i = 0; i < casesMap.size(); i++) {
                //if (casesMap[i] != nullptr) {
                    math::Vec3f point = casesMap[i].getCellVolume().getCenter();
                    
                    math::Vec3f coordsCaseP = getCoordinatesAt(point);
                    //std::cout<<"point : "<<coordsCaseP<<std::endl;
                    minX = (coordsCaseP.x() < minX) ? coordsCaseP.x() : minX;
                    minY = (coordsCaseP.y() < minY) ? coordsCaseP.y() : minY;
                    minZ = (coordsCaseP.z() < minZ) ? coordsCaseP.z() : minZ;
                    maxX = (coordsCaseP.x() > maxX) ? coordsCaseP.x() : maxX;
                    maxY = (coordsCaseP.y() > maxY) ? coordsCaseP.y() : maxY;
                    maxZ = (coordsCaseP.z() > maxZ) ? coordsCaseP.z() : maxZ;
                    //std::cout<<"minX : "<<minX<<std::endl<<"minY : "<<minY<<std::endl<<"minZ : "<<minZ<<std::endl<<"maxX : "<<maxX<<std::endl<<"maxY : "<<maxY<<std::endl<<"maxZ : "<<maxZ<<std::endl;

                    nbCases++;
                //}
            }
            if (nbCases == 0) {
                minX = minY = minZ = maxX = maxY = maxZ = 0;
            }
            //////////std::cout<<"mins : "<<minX<<","<<minY<<","<<minZ<<std::endl<<"maxs : "<<maxX<<","<<maxY<<","<<maxZ<<std::endl;
            //system("PAUSE");
        }
        template<typename Object>
        math::Vec3f GridMap<Object>::getSize() {
            
            return (casesMap.size() == 0) ? math::Vec3f(0, 0, 0) : math::Vec3f (maxX - minX+1, maxY - minY+1, maxZ - minZ+1);
        }
        template<typename Object>
        vector<GridCell<Object>*> GridMap<Object>::getNeightbours(Object object, GridCell<Object> *cell, bool getCellOnPassable) {
            math::Vec3f coords = cell->getCoords();
            vector<GridCell<Object>*> neightbours;
            for (int i = coords.x() - 1; i <= coords.x() + 1; i++) {
                for (int j = coords.y() - 1; j <= coords.y() + 1; j++) {
                    for (int k = coords.z() - 1; k <= coords.z() + 1; k++) {
                        if (!(i == coords.x() && j == coords.y() && k == coords.z())) {
                            math::Vec2f neightbourCoords(i, j);
                            GridCell<Object> *neightbour = getGridCellAtFromCoords(neightbourCoords);
                            if (neightbour != nullptr) {
                                if (getCellOnPassable)
                                    neightbours.push_back(neightbour);
                                else {                                    
                                    if (neightbour->isPassable()) {
                                        neightbours.push_back(neightbour);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            return neightbours;
        }
        template<typename Object>
        GridCell<Object>* GridMap<Object>::getGridCellAtFromCoords(math::Vec3f coords) {
            int indice = (math::Math::abs(minX) + coords.x()) + (math::Math::abs(minY) + coords.y()) * nbCasesPerRow + (math::Math::abs(minZ) + coords.z()) * nbCasesPerCol;
            if (indice >= 0 && indice < static_cast<int>(casesMap.size()))
                return &casesMap[indice];
            return nullptr;
        }
        template<typename Object>
        void GridMap<Object>::clear() {
            /*for (unsigned int i = 0; i < casesMap.size(); i++) {
                 if (casesMap[i] != nullptr)
                    delete casesMap[i];
            }*/
            casesMap.clear();
        }
        template<typename Object>
        GridMap<Object>::~GridMap () {
            /*vector<Entity*> entities = getEntities();
            for (unsigned int i = 0; i < entities.size(); i++) {
                delete entities[i];
            }*/
            clear();
        }
    }
}


