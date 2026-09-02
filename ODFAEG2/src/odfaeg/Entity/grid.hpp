#ifndef ODFAEG_GRIDMAP_HPP
#define ODFAEG_GRIDMAP_HPP
#include "../Math/vec.hpp"
#include "../Physics/boundingBox.hpp"
#include "gridCell.hpp"
namespace odfaeg {
    namespace entity {
        template <typename Object>
        class GridMap {
            public:
            GridMap (float cellWidth, float cellHeight, float cellDepth);
            int getCellWidth();
            int getCellHeight();            
            int getCellDepth();
            int getNbCasesPerRow ();
            int getNbCasesPerCol ();
            /*void setBaseChangementMatrix(math::BaseChangementMatrix bm);
            math::BaseChangementMatrix getBaseChangementMatrix();*/
            bool addEntity (Object entity, physic::BoundingBox objectVolume);
            bool containsEntity(Object entity, math::Vec3f pos);
            Object* getEntity (int id);
            Object* getEntity (std::string name);
            void createCellMap (math::Vec3f &point);
            void replaceEntity (Object entity);
            //Supprime une tile dans la cellule. (Sans la supprimer de la m�moire.)
            bool removeEntity (Object entity, physic::BoundingBox volume);
            bool deleteEntity (Object object, physic::BoundingBox volume);
            bool deleteEntity(int id);
            void removeCellMap (GridCell<Object> *cell);
            vector<GridCell<Object>*> getCasesInBox (physic::BoundingBox bx);
            vector<Object> getEntitiesInBox(physic::BoundingBox box);
            vector<Object> getEntities ();
            math::Vec3f getMins ();
            GridCell<Object>* getGridCellAt (math::Vec3f point);            
            math::Vec3f getCoordinatesAt(math::Vec3f &point);
            std::vector<GridCell<Object>> getCasesMap ();
            void checkExts ();
            math::Vec3f getSize();
            vector<GridCell<Object>*> getNeightbours(Object object, GridCell<Object> *cell, bool getCellOnPassable);
            GridCell<Object>* getGridCellAtFromCoords(math::Vec3f coords);
            void clear();
            ~GridMap ();
            private:
            std::vector<GridCell<Object>> casesMap;
            int nbCasesPerRow, nbCasesPerCol;
            int minX, minY, minZ, maxX, maxY, maxZ;
            float cellWidth, cellHeight, cellDepth;
        };
    }
}
#include "grid.inl"
#endif