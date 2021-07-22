#ifndef ODFAEG_ENTITY_MANAGER_2D_HPP
#define ODFAEG_ENTITY_MANAGER_2D_HPP
#include "../Core/utilities.h"
#include "../Physics/boundingBox.h"
#include "entity.h"
#include "baseChangementMatrix.h"
#include "2D/wall.h"
#include "cellMap.h"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace graphic {
        /**
          * \file entityManager.h
          * \class EntityManager
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          * \brief Abstract class of every 2D entity managers.
          */
        class ODFAEG_GRAPHICS_API SceneManager {
        public :
            /**
            * \fn std::vector<Entity*> getEntities(std::string expression)
            * \brief virtual function to redefine to get the entities in the world, depending on the given expression.
            * \param an expression which determine which entities types to get.
            */
            virtual std::vector<Entity*> getEntities(std::string expression) = 0;
            virtual std::vector<Entity*> getRootEntities(std::string expression) = 0;
            virtual std::vector<Entity*> getChildrenEntities(std::string expression) = 0;
            /**
            * \fn void checkVisibleEntities()
            * \brief virtual function to redefine to check visible entities.
            * \param an expression which determine which entities types to get.
            */
            virtual void checkVisibleEntities (EntityFactory& factory) = 0;
            /**
            * \fn std::vector<Entity*>  getVisibleEntities (std::string expression)
            * \brief get the visible entities.
            * \return std::vector<Entity*> the visible entities.
            */
            virtual std::vector<Entity*> getVisibleEntities (std::string expression, EntityFactory& factory)  = 0;
            virtual std::vector<Entity*> getEntitiesInBox(physic::BoundingBox rect, std::string expression) = 0;
            /**
            * \fn bool collide (Entity* entity)
            * \brief virtual function to redefine if an entity can be in collision with another one.
            * \param Entity* entity : the entity to collide with.
            * \return true if entities collide.
            */
            virtual bool collide (Entity* entity) = 0;
            virtual bool collide (Entity* entity, math::Vec3f position) = 0;
            virtual bool collide (Entity* entity, math::Ray ray) = 0;
            /**
            * \fn void generate_map(std::vector<Tile*> tGrounds, std::vector<Tile*> tWalls, BoundingBox& zone) = 0;
            * \brief virtual method used to generate the map.
            * \param std::vector<Tile*> tGrounds : the tiles of the ground to generate.
            * \param std::vector<Tile*> tWalls : the tiles of the walls to generate.
            * \param BoundingZone& zone : the zone of the area where to generate the map.
            */
            virtual void generate_map(std::vector<Tile*> tGrounds, std::vector<g2d::Wall*> tWalls, math::Vec2f tileSize, physic::BoundingBox& zone, bool terrain3D, EntityFactory& factory) = 0;
            /**
            * \fn void moveEntity(Entity* entity, float x, float y, float z)
            * \brief virtual method to redefine to move an entity to the world.
            * \param Entity* entity : the entity to move.
            * \param float x : the translation in x.
            * \param float y : the translation in y.
            * \param float z : the translation in z.
            */
            virtual void moveEntity(Entity *entity, float x, float y, float z) = 0;
            /**
            * \fn bool addEntity(Entity* entity)
            * \brief virtual function to redefine to add an entity into the manager.
            * \param Entity* entity : the entity to add.
            */
            virtual bool addEntity(Entity *entity) = 0;
            /**
            * \fn void drawOnComponents(std::string expression, int layer, sf::BlendMode mode)
            * \brief virtual method to redefine to draw the visibles entities to a component.
            * \param std::string expression : the expression which determine the entities to draw.
            * \param int layer : the number of the layer of the component.
            * \param sf::BlendMode mode : the blend mode.
            */
            virtual void drawOnComponents(std::string expression, int layer, sf::BlendMode mode) = 0;
            /**
            * \fn void drawOnComponents(std::string expression, int layer, sf::BlendMode mode)
            * \brief virtual method to redefine to draw a drawable object to a component.
            * \param Drawable the drawable object to draw.
            * \param int layer : the number of the layer of the component.
            * \param RenderStates states : the render states.
            */
            virtual void drawOnComponents(Drawable &drawable, int layer, RenderStates states) = 0;
            /**
            *   \fn BaseChangementMatrix getBaseChangementMatrix()
            *   \brief get the base changement matrix of the manager.
            *   \return the base changement matrix.
            */
            virtual BaseChangementMatrix getBaseChangementMatrix() = 0;
            /**
            *   \fn std::vector<Vec3f> getPath(Entity* entity, Vec3f finalPos)
            *   \brief virtual method to redefine to get the path between an entity and a destination.
            *   \param Entity* entity : the entity.
            *   \param Vec3f finalPos : the destination.
            */
            virtual std::vector<math::Vec3f> getPath(Entity* entity, math::Vec3f finalPos) = 0;
            /**
            *   \fn CellMap<Entity>* getGridCellAt(Vec3f pos)
            *   \brief virtual method to redefine to get a cell at the given position containing the entities.
            *   \
            */
            virtual CellMap* getGridCellAt(math::Vec3f pos) = 0;
            /**
            *   \fn std::vector<CellMap<Entity>*> getCasesMap()
            *   \brief virtual method to redefine to get all the cells containing the entities.
            *   \return std::vector<CellMap<Entity>*> all the cells containing the entities.
            */
            virtual math::Vec3f getCoordinatesAt(math::Vec3f point) = 0;
            virtual std::vector<CellMap*> getCasesMap() = 0;
            virtual bool removeEntity(Entity* entity) = 0;
            virtual bool deleteEntity(Entity* entity) = 0;
            virtual Entity* getEntity(int id) = 0;
            virtual Entity* getEntity(std::string name) = 0;
            virtual std::string getName() = 0;
            virtual void setName(std::string name) = 0;
        };
    }
}
#endif // ENTITY_MANAGER
