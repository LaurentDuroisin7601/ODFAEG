namespace odfaeg {
    namespace entity {
        template <typename Object>
        class GridMap {
            GridMap (int cellWidth, int cellHeight, int cellDepth);
            int getCellWidth();
            int getCellHeight();            
            int getCellDepth();
            int getNbCasesPerRow ();
            int getNbCasesPerCol ();
            void setBaseChangementMatrix(BaseChangementMatrix bm);
            BaseChangementMatrix getBaseChangementMatrix();
            bool addEntity (Object entity, physic::BoundingVolume objectVolume);
            bool containsEntity(Object entity, math::Vec3f pos);
            Object* getEntity (int id);
            Entity* getEntity (std::string name);
            void createCellMap (math::Vec3f &point);
            void replaceEntity (Object entity);
            //Supprime une tile dans la cellule. (Sans la supprimer de la m�moire.)
            bool removeEntity (Object entity, physic::BoundingBox volume);
            bool deleteEntity (Object object, physic::BoundingBox volume);
            bool deleteEntity(int id);
            void removeCellMap (GridCell *cell);
            vector<GridCell*> getCasesInBox (physic::BoundingBox bx);
            vector<Object> GridMap::getEntitiesInBox(physic::BoundingBox box);
            vector<Object> GridMap<Object>::getEntities ();
            math::Vec3f GridMap<Object>::getMins ();
            CellMap* GridMap<Object>::getGridCellAt (math::Vec3f point);            
            math::Vec3f GridMap<Object>::getCoordinatesAt(math::Vec3f &point)
            std::vector<GridCell*> GridMap<Object>::getCasesMap ();
            void GridMap<Object>::checkExts ();
            math::Vec3f GridMap<Object>::getSize();
            vector<GridCell*> GridMap<Object>::getNeightbours(Object object, GridCell *cell, bool getCellOnPassable);
            GridCell* getGridCellAtFromCoords(math::Vec3f coords);
            ~GridCell ();
        };
    }
}