#ifndef ODFAEG_ECS_GRID_HPP
#define ODFAEG_ECS_GRID_HPP
#include "cell.hpp"
#include "../../Math/vec4.h"
#include "../baseChangementMatrix.h"
#include "../../Physics/boundingVolume.h"

namespace odfaeg {
    namespace graphic {
        namespace ecs {

            class Grid {
                public :
                    /**
                    * \fn  CellMap<Entity>* getGridCellAtFromCoords(Vec3f coords)
                    * \brief get the cell at a given position.
                    * \param Vec3f coords : the coordinates of the cell.
                    */
                    Cell* getGridCellAtFromCoords(math::Vec3f coords);
                    /**
                    * \fn GridMap(int cellWith, int cellHeight, int nbLayers)
                    * \brief constructor.
                    * \param cellWidth : the width of the cells.
                    * \param cellHeight : the height of the cells.
                    * \param nbLayers : the depth of the cells.
                    */
                    Grid (int cellWidth=100, int cellHeight=50, int cellDepth=0);
                    /** \fn  ~GridMap ();
                    *   \brief destructor.
                    */
                    ~Grid ();
                    /**
                    * \fn bool addEntity (Entity *entity);
                    * \brief add an entity into the grid.
                    * \param Entity* entity : the entity to add.
                    * \return if the entity has been successfully added.
                    */
                    bool addEntity (physic::BoundingBox globalBounds, EntityId entity);
                    /**
                    * \fn Entity* getEntity (int id);
                    * \brief get an entity depending on the given id.
                    * \param id : the id of the entity.
                    * \return the entity.
                    */
                    EntityId getEntity (ComponentMapping mapping, int id);
                    //EntityId getEntity (std::string name);
                    /**
                    * \fn deleteEntity(Entity* entity)
                    * \brief remove an entity from the grid and from the memory.
                    * \param entity : the entity to delete.
                    * \return if the entity has been successfully deleted.
                    */
                    bool removeEntity(physic::BoundingBox globalBounds, EntityId entity);
                    /**
                    * \fn  bool collideWithEntity(Entity* entity);
                    * \brief check if an entity collide with the entity of the grid.
                    * \param entity : check if the entity is colliding with an entity (or a cell) of the grid.
                    * \return if the entity is in collision with the grid.
                    */
                    std::vector<Cell*> getNeightbours(EntityId entity, Cell *cell, bool getCellOnPassable);
                    bool collideWithEntity(ComponentMapping& componentMapping, EntityId entity);
                    bool collideWithEntity(ComponentMapping& mapping, EntityId entity, math::Vec3f position);
                    /**
                    * \fn std::vector<Entity*> getEntities()
                    * \brief return every entities which are stored into each cells of the grid.
                    */
                    std::vector<EntityId> getEntities ();
                    /**
                    * \fn  std::vector<CellMap<Entity>*> getCasesMap()
                    * \brief get every cells of the map.
                    * \return the cells.
                    */
                    std::vector<Cell*> getCells ();
                    /**
                    * \fn std::vector<CellMap<Entity>*> getCasesInBox(BoundingBox bx)
                    * \brief return every cells which are in the given bounding box.
                    * \param bx : the bounding box.
                    * \return the cells in the bounding box.
                    */
                    std::vector<Cell*> getCellsInBox(physic::BoundingBox bx);
                    /**
                    * \fn bool  containsEntity (Entity *entity, Vec3f pos);
                    * \brief check if the grid contains the entity at the given pos.
                    * \param entity : the entity.
                    * \param Vec3f pos : the given position.
                    */
                    bool  containsEntity (EntityId entity, math::Vec3f pos);
                    /**
                    * \fn std::vector<Entity*> getEntitiesInBox(BoundingBox bx)
                    * \brief get the entities which are in the given bounding box.
                    * \param bx : the bounding box.
                    * \return std::vector<Entity*> : the entities.
                    */
                    std::vector<EntityId> getEntitiesInBox(ComponentMapping& mapping, physic::BoundingBox box);
                    /**
                    * \fn  CellMap<Entity>* getGridCellAt (Vec3f point)
                    * \brief get the cell at the given position.
                    * \param point : the point.
                    * \return the cell.
                    */
                    Cell* getGridCellAt (math::Vec3f point);
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
                    void createCell(math::Vec3f &point);
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
                    int getNbCellsPerRow ();
                    /**
                    * \fn int getNbCasesPerCol ();
                    * \brief return the number of cases per col.
                    * \return the number of cases per col.
                    */
                    int getNbCellsPerCol ();
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
                    void removeCell (Cell *cell);
                private :
                    std::vector<Cell*> cells; /**Cells of the grid, I use a raw pointer here because, the grid is dynamic so I have to copy the pointers.*/
                    int nbCellsPerRow, nbCellsPerCol, /**> the number of cases per row, the number of cases per columns*/
                    minX, minY, minZ, maxX, maxY, maxZ, /**> the mins and maxs of the grid.*/
                    cellWidth, cellHeight, cellDepth, /**> the dimentions of the cells of the grid.*/
                    offsetX, offsetY, offsetZ; /**> the offsets of the cells of the grid.*/
            };
        }
    }
}
#endif
