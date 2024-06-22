#ifndef ODFAEG_GRID_2D_H
#define ODFAEG_GRID_2D_H
#include <vector>
#include "tile.h"
#include "../Physics/boundingBox.h"
#include "cellMap.h"
#include "baseChangementMatrix.h"
#include "../Math/computer.h"
#include "../Physics/collisionResultSet.hpp"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace graphic {
             /**
              * \file gridMap.h
              * \class GridMap
              * \author Duroisin.L
              * \version 1.0
              * \date 1/02/2014
              * \brief a grid used to partitionate the world.
              */
        class GridMap {
            public :
            /**
            * \fn std::vector<CellMap<Entity>*> getNeightbours(Entity* entity, CellMap<Entity> *cell, bool getCellOnPassable);
            * \brief get the neightbour cells of the given cell.
            * \param Entity* entity : cell.
            * \param bool getCellOnPassable : if we want to get the cells which are passable or not.
            * \return the neightbour cells.
            */
            std::vector<CellMap*> getNeightbours(Entity* entity, CellMap *cell, bool getCellOnPassable);
            /**
            * \fn std::vector<Vec3f> getPath (Entity* entity, Vec3f finalPos);
            * \brief get the path between an entity and a destination
            * \param Entity* entity : an entity.
            * \param Vec3f finalPos : the destination.
            * \return the points of the curve of the path.
            */
            std::vector<math::Vec3f> getPath (Entity* entity, math::Vec3f finalPos);
            /**
            * \fn  CellMap<Entity>* getGridCellAtFromCoords(Vec3f coords)
            * \brief get the cell at a given position.
            * \param Vec3f coords : the coordinates of the cell.
            */
            CellMap* getGridCellAtFromCoords(math::Vec3f coords);
            /**
            * \fn GridMap(int cellWith, int cellHeight, int nbLayers)
            * \brief constructor.
            * \param cellWidth : the width of the cells.
            * \param cellHeight : the height of the cells.
            * \param nbLayers : the depth of the cells.
            */
            GridMap (int cellWidth, int cellHeight, int cellDepth);
            /** \fn  ~GridMap ();
            *   \brief destructor.
            */
            ~GridMap ();
            /**
            * \fn bool addEntity (Entity *entity);
            * \brief add an entity into the grid.
            * \param Entity* entity : the entity to add.
            * \return if the entity has been successfully added.
            */
            bool addEntity (Entity *entity);
            /**
            * \fn Entity* getEntity (int id);
            * \brief get an entity depending on the given id.
            * \param id : the id of the entity.
            * \return the entity.
            */
            Entity* getEntity (int id);
            Entity* getEntity (std::string name);
            /**
            * \fn void replaceEntity (Entity* entity)
            * \brief replace an entity in the grid.
            * \param entity : the entity to replace.
            */
            void replaceEntity (Entity *entity);
            /**
            * \fn deleteEntity(Entity* entity)
            * \brief remove an entity from the grid and from the memory.
            * \param entity : the entity to delete.
            * \return if the entity has been successfully deleted.
            */
            bool deleteEntity(Entity *entity);
            /**
            * \fn deleteEntity(Entity* entity)
            * \brief remove an entity from the grid and from the memory by its id.
            * \param id : the id of the entity.
            * \return if the entity has been successfully deleted.
            */
            bool deleteEntity (int id);
            /**
            * \fn removeEntity(Entity* entity)
            * \brief remove an entity from the grid.
            * \param entity : the entity.
            * \return if the entity has been successfully removed.
            */
            bool removeEntity(Entity *entity);
            /**
            * \fn  bool collideWithEntity(Entity* entity);
            * \brief check if an entity collide with the entity of the grid.
            * \param entity : check if the entity is colliding with an entity (or a cell) of the grid.
            * \return if the entity is in collision with the grid.
            */
            bool collideWithEntity(Entity* entity);
            bool collideWithEntity(Entity* entity, math::Vec3f position);
            /**
            * \fn std::vector<Entity*> getEntities()
            * \brief return every entities which are stored into each cells of the grid.
            */
            std::vector<Entity*> getEntities ();
            /**
            * \fn  std::vector<CellMap<Entity>*> getCasesMap()
            * \brief get every cells of the map.
            * \return the cells.
            */
            std::vector<CellMap*> getCasesMap ();
            /**
            * \fn std::vector<CellMap<Entity>*> getCasesInBox(BoundingBox bx)
            * \brief return every cells which are in the given bounding box.
            * \param bx : the bounding box.
            * \return the cells in the bounding box.
            */
            std::vector<CellMap*> getCasesInBox(physic::BoundingBox bx);
            /**
            * \fn bool  containsEntity (Entity *entity, Vec3f pos);
            * \brief check if the grid contains the entity at the given pos.
            * \param entity : the entity.
            * \param Vec3f pos : the given position.
            */
            bool  containsEntity (Entity *entity, math::Vec3f pos);
            /**
            * \fn std::vector<Entity*> getEntitiesInBox(BoundingBox bx)
            * \brief get the entities which are in the given bounding box.
            * \param bx : the bounding box.
            * \return std::vector<Entity*> : the entities.
            */
            std::vector<Entity*> getEntitiesInBox(physic::BoundingBox bx);
            /**
            * \fn  CellMap<Entity>* getGridCellAt (Vec3f point)
            * \brief get the cell at the given position.
            * \param point : the point.
            * \return the cell.
            */
            CellMap* getGridCellAt (math::Vec3f point);
            /**
            * \fn Vec3f getCoordinatesAt (Vec3f &point);
            * \brief get the cell's coordinates at the given pos.
            * \param point : the position.
            * \return Vec3f the coordinates.
            */
            math::Vec3f getCoordinatesAt (math::Vec3f &point);
            /**
            * \fn void createCellMap(Vec3f &point);
            * \brief create a cell a the given position.
            * \param point : the position.
            */
            void createCellMap(math::Vec3f &point);
            /**
            * \fn Vec3f getMins ()
            * \brief return the minimums of the grid.
            * \return the minimums of the grid.
            */
            math::Vec3f getMins ();
            /**
            * \fn Vec3f getSize ();
            * \brief return the size of the grid.
            * \return Vec3f the size.
            */
            math::Vec3f getSize ();
            /**
            * \fn int getNbCasesPerRow ();
            * \brief return the number of cases per row.
            * \return the number of cases per row.
            */
            int getNbCasesPerRow ();
            /**
            * \fn int getNbCasesPerCol ();
            * \brief return the number of cases per col.
            * \return the number of cases per col.
            */
            int getNbCasesPerCol ();
            /**
            * \fn void setBaseChangementMatrix (BaseChangementMatrix bm)
            * \brief change the base changement matrix.
            * \param bm : the base changement matrix.
            */
            void setBaseChangementMatrix (BaseChangementMatrix bm);
            /**
            * \fn BaseChangementMatrix getBaseChangementMatrix()
            * \brief get the base changement matrix.
            * \return the base changement matrix.
            */
            BaseChangementMatrix getBaseChangementMatrix();
            /**
            * \fn int getCellDepth();
            * \brief get the depth of the cell.
            * \return the depth of the cell.
            */
            int getCellDepth();
            /**
            * \fn int getCellWidth();
            * \brief get the width of the cell.
            * \return the with of the cell.
            */
            int getCellWidth ();
            /**
            * \fn int getCellHeight();
            * \brief get the height of the cell.
            * \return the height of the cell.
            */
            int getCellHeight ();
            int getOffsetX() {
                return offsetX;
            }
            int getOffsetY() {
                return offsetY;
            }
            int getOffsetZ() {
                return offsetZ;
            }
            private:
            BaseChangementMatrix bm; /**> the base changement matrix.*/
            /**
            * \fn void checkExts()
            * \brief check the extremity of the grid.
            */
            void checkExts();
            /**
            * \fn void removeCellMap (CellMap<Entity> *cell)
            * \brief remove a cell from the grid and from the memory.
            * \param cell : the cell to remove.
            */
            void removeCellMap (CellMap *cell);
            std::vector<CellMap*> casesMap; /**> the cells of the grid.*/
            int nbCasesPerRow, nbCasesPerCol, /**> the number of cases per row, the number of cases per columns*/
            minX, minY, minZ, maxX, maxY, maxZ, /**> the mins and maxs of the grid.*/
            cellWidth, cellHeight, cellDepth, /**> the dimentions of the cells of the grid.*/
            offsetX, offsetY, offsetZ; /**> the offsets of the cells of the grid.*/
        };
    }
}
#endif
