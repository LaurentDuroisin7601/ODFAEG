#ifndef ODFAEG_ECS_SCENE_HPP
#define ODFAEG_ECS_SCENE_HPP
#include "sceneManager.hpp"
#include "modelFactory.hpp"
namespace odfaeg {
    namespace graphic {
        namespace ecs {
            class ODFAEG_GRAPHICS_API Scene : public SceneManager {

                public :
                    /** \fn Map(RenderComponentManager* frcm, std::string name, int cellWidth, int cellHeight, int cellDepth);
                    *   \brief constructor
                    *   \param frcm : the component manager used to manage components.
                    *   \param name : the name of the map.
                    *   \param cellWidth : the width of the cells.
                    *   \param cellHeight : the height of the cells.
                    *   \param cellDepth : the depth of the cells.
                    */

                    Scene();
                    Scene(RenderComponentManager* frcm, std::string name, int cellWidth, int cellHeight, int cellDepth);
                    Grid *gridMap; /**> The grid used to store the entities.*/
                    /**
                    * \fn int getId()
                    * \brief get the id of the map.
                    * \return the id.
                    */
                    int getId();
                    /**
                    * \fn getVersion()
                    * \brief get the version of the map.
                    * \return the version.
                    */
                    int getVersion();
                    /**
                    * \fn std::string getName()
                    * \brief get the name of the map.
                    * \return the name.
                    */
                    std::string getName();
                    /**
                    * \fn void setVersion(int version);
                    * \brief set the version of the map.
                    * \param version : the version.
                    */
                    void setVersion(int version);
                    /**
                    * \fn void setName(std::string name);
                    * \brief set the name of the map.
                    * \param std::string name : the name.
                    */
                    void setName (std::string name);
                    /**
                    * \fn void setName(std::string name);
                    * \brief set the name of the map.
                    * \param std::string name : the name.
                    */
                    void setId (int id);
                    /**
                    * \fn getCompImage(const void* resource)
                    * \brief get the number of time that the given resource is used on the map.
                    * \param resource : the resource.
                    * \return how many times the resource is used.
                    */
                    int getCompImage(const void* resource);
                    /**
                    * \fn math::Vec3f getPosition()
                    * \brief get the position of a map.
                    * \return the position of the map.
                    */
                    math::Vec3f getPosition();
                    /**
                    * \fn int getWidth();
                    * \brief get the width of the map.
                    * \return the width of the map.
                    */
                    int getWidth();
                    /**
                    * \fn int getHeight();
                    * \brief get the height of the map.
                    * \return the height of the map.
                    */
                    int getHeight();
                    /**
                    * \fn bool addEntity(Entity *entity);
                    * \brief add an entity into the map.
                    * \param entity : the entity to add.
                    * \return if the entity has been successfully added.
                    */
                    bool addEntity(ComponentMapping& componentMapping, EntityId entity);
                    /**
                    * \fn bool removeEntity(Entity *entity);
                    * \brief remove an entity from the map.
                    * \param entity : the entity to remove.
                    * \return if the entity has been successfully removed.
                    */
                    bool removeEntity (ComponentMapping& componentMapping, EntityId entity);
                    bool deleteEntity(ComponentMapping& componentMapping, EntityId entity);
                    /**
                    * \fn int getNbCasesPerRow ();
                    * \brief get the number of cases per row.
                    * \return the number of cells per row.
                    */
                    int getNbCasesPerRow ();
                    /**
                    * \fn void rotateEntity(Entity *entity, int angle);
                    * \brief rotate an entity and replace it to the grid.
                    * \param entity : the entity.
                    * \param angle : the angle.
                    */
                    void rotateEntity(EntityId entity, int angle);
                    /**
                    * \fn void scaleEntity(Entity *entity, int angle);
                    * \brief scale an entity and replace it to the grid.
                    * \param entity : the entity.
                    * \param sx : the x factor.
                    * \param sy : the y factor.
                    */
                    void scaleEntity(EntityId entity, float sx, float sy, float sz);
                    /**
                    * \fn void moveEntity(Entity *entity, float dx, float dy, float dz);
                    * \brief move an entity and replace it to the grid.
                    * \param dx : the x translation.
                    * \param dy : the y translation.
                    * \param dz : the z translation.
                    */
                    void moveEntity(ComponentMapping& componentMapping, EntityId entity, float dx, float dy, float dz);
                    void scaleEntity(ComponentMapping& componentMapping, EntityId entity, float sx, float sy, float sz);
                    void rotateEntity(ComponentMapping& componentMapping, EntityId entity, int angle);
                    /**
                    * \fn  void checkVisibleEntities();
                    * \brief check the entities which are visible. (Which are in the field of view of the camera)
                    */
                    void checkVisibleEntities(EntityFactory& factory, ComponentMapping& mapping);
                    /**
                    * \fn  std::vector<CellMap<Entity>*> getCasesMap();
                    * \brief get every cells of the grid.
                    * \return the cells of the grid.
                    */
                    std::vector<Cell*> getCasesMap();
                    template <typename Archive>
                    void serialize(Archive& ar) {
                        /*if (!ar.isInputArchive()) {
                            ar(name);
                            ar(cellWidth);
                            ar(cellHeight);
                            ar(cellDepth);
                            BaseChangementMatrix bcm = getBaseChangementMatrix();
                            ar(bcm);
                            std::vector<Entity*> entities = getRootEntities("*");
                            std::vector<Entity*> internalEntities;
                            for (unsigned int i = 0; i < entities.size(); i++) {
                                if (!entities[i]->isExternal()) {
                                    internalEntities.push_back(entities[i]);
                                }
                            }
                            ar(internalEntities);
                            std::vector<CellMap*> cells = getCasesMap();
                            unsigned int nb = 0;
                            for (unsigned int i = 0; i < cells.size(); i++) {
                                if (cells[i] != nullptr) {
                                        nb++;
                                }
                            }
                            ar(nb);
                            for (unsigned int i = 0; i < cells.size(); i++) {
                                if (cells[i] != nullptr) {
                                    math::Vec3f center = cells[i]->getCenter();
                                    bool isPassable = cells[i]->isPassable();
                                    ar(center);
                                    ar(isPassable);
                                }
                            }
                        } else {
                            ar(name);
                            ar(cellWidth);
                            ar(cellHeight);
                            ar(cellDepth);
                            gridMap = new GridMap(cellWidth, cellHeight, cellDepth);
                            BaseChangementMatrix bcm;
                            //std::cout<<"read bcm"<<std::endl;
                            ar(bcm);
                            //std::cout<<"bcm read"<<std::endl;
                            setBaseChangementMatrix(bcm);
                            std::vector<Entity*> entities;
                            //std::cout<<"read entities"<<std::endl;
                            ar(entities);
                            //std::cout<<"size : "<<entities.size()<<std::endl;
                            for (unsigned int i = 0; i < entities.size(); i++) {
                                //std::cout<<"add entity"<<std::endl;
                                addEntity(entities[i]);
                            }
                            unsigned int size;
                            ar(size);
                            //std::cout<<"cases maps : "<<std::endl;
                            for (unsigned int i = 0; i < size; i++) {
                                math::Vec3f center;
                                ar(center);
                                //std::cout<<"center : "<<center<<std::endl;
                                bool isPassable;
                                ar(isPassable);
                                //std::cout<<"passable : "<<isPassable<<std::endl;
                                CellMap* cell = getGridCellAt(center);
                                if (cell != nullptr) {
                                    cell->setPassable(isPassable);
                                }
                            }
                        }*/
                    }
                    void setRenderComponentManager(RenderComponentManager* rcm);
                    /**
                    * \fn std::vector<Entity*> getEntities(std::string type)
                    * \brief get the entities of the given types.
                    * \param std::string type : the types.
                    * \return the entities.
                    */
                    std::vector<EntityId> getEntities(ComponentMapping& componentMapping, std::string type);
                    std::vector<EntityId> getRootEntities(ComponentMapping& componentMapping,std::string type);
                    std::vector<EntityId> getChildrenEntities(ComponentMapping& componentMapping,std::string type);
                    /**
                    * \fn std::vector<Entity*> getVisibleEntities(std::string type)
                    * \brief get the visible entities of the given types.
                    * \param std::string type : the types.
                    * \return the visible entities.
                    */
                    std::vector<EntityId> getVisibleEntities (std::string type, EntityFactory& factory, ComponentMapping& componentMapping);
                    /**
                    * \fn std::vector<Entity*> getEntitiesInBox(BoundingBox bx, std::string types)
                    * \brief get the entities of the given types which are into the given box.
                    * \param BoundingBox bx : the bounding box of the entity.
                    * \param std::string type : the types.
                    * \return the entities.
                    */
                    std::vector<EntityId> getEntitiesInBox (ComponentMapping& componentMapping, physic::BoundingBox bx, std::string types);
                    /**
                    * \fn math::Vec3f getCoordinatesAt(math::Vec3f p)
                    * \brief get coordinates at a given point.
                    * \param math::Vec3f point : the point.
                    * \return the coordinates.
                    */
                    math::Vec3f getCoordinatesAt(math::Vec3f p);
                    /**
                    * \fn CellMap<Entity>* getGridCellAt(math::Vec3f p)
                    * \brief get the cell at a given point.
                    * \param math::Vec3f point : the point.
                    * \return the cell.
                    */
                    Cell* getGridCellAt(math::Vec3f p);
                    /**
                    * \fn std::vector<CellMap<Entity>>* getGridCellAt(math::Vec3f p)
                    * \brief get the cells which are in the given box.
                    * \param BoundingBox bx: the box.
                    * \return the cells.
                    */
                    std::vector<Cell*> getCasesInBox (physic::BoundingBox bx);
                    /**
                    * \fn bool collide (Entity *entity)
                    * \brief check if an entity collide with another one.
                    * \param Entity* entity : the entity.
                    * \return if an entity collide with another.
                    */
                    bool collide(ComponentMapping& componentMapping, EntityId entity);
                    bool collide(ComponentMapping& componentMapping, EntityId entity, math::Vec3f position);
                    bool collide(ComponentMapping& componentMapping, EntityId entity, math::Ray ray);
                    void generate_labyrinthe(ComponentMapping& mapping, std::vector<EntityId> tGround, std::vector<EntityId> walls, math::Vec2f tileSize, physic::BoundingBox &box, EntityFactory& factory);
                    /** \fn generate_map(std::vector<Tile*> tGround, std::vector<Tile*> walls, BoundingBox &box)
                    *   \brief generate a map in the given zone, which the given tiles for the ground and the given tiles for the walls.
                    *   \param std::vector<Tile*> tGround : the tiles used for the ground.
                    *   \param std::vector<Tile*> walls : the tiles used for the walls. (The walls are placed to the edges of the map)
                    */
                    void generate_map(ComponentMapping& mapping, std::vector<EntityId> tGround, std::vector<EntityId> walls, math::Vec2f tileSize, physic::BoundingBox &box, EntityFactory& factory);
                    /**
                    *   \fn void getChildren (Entity *entity, std::vector<Entity*> &entities, std::string type);
                    *   \brief get the children of a kind of types from an entity.
                    *   \param entity : the entity.
                    *   \param entities : the children entities.
                    *   \param type : the types.
                    */
                    void getChildren (ComponentMapping& componentMapping, EntityId entity, std::vector<EntityId> &entities, std::string type);
                    void drawOnComponents(std::string expression, int layer, sf::BlendMode mode = sf::BlendAlpha);
                    /**
                    * \fn void drawOnComponents(Drawable& drawable, int layer, RenderStates states = RenderStates::Default);
                    * \brief draw a drawable object onto a component.
                    * \param layer : the layer of the component on which to draw the entities.
                    * \param states : the render states.
                    */
                    void drawOnComponents(Drawable& drawable, int layer, RenderStates states = RenderStates::Default);
                    /**
                    * \fn BaseChangementMatrix getBaseChangementMatrix();
                    * \brief get the base changement matrix of the map.
                    * \return the base changement matrix.
                    */
                    BaseChangementMatrix getBaseChangementMatrix();
                    void setBaseChangementMatrix(BaseChangementMatrix bm);
                    int getCellWidth();
                    int getCellHeight();
                private :
                    /**
                    * \fn void increaseComptImg (const void* resource);
                    * \brief increase the counter for the given resource.
                    * \param resource : the resource.
                    */
                    void increaseComptImg (const void* resource);
                    /**
                    * \fn void decreaseComptImg (const void* resource);
                    * \brief decrease the counter for the given resource.
                    * \param resource : the resource.
                    */
                    void decreaseComptImg (const void* resource);
                    /**
                    * \fn void removeComptImg (const void* resource);
                    * \brief remove the counter for the given resource.
                    * \param resource : the resource.
                    */
                    void removeComptImg (const void* resource);
                    std::string name; /**> the name of the map.*/
                    int id, version; /**> the version of the map.*/
                    Scene (const Scene &other); /**> A map is not copiable.*/
                    Scene operator= (const Scene &other); /**> A map is not affectable.*/
                    std::map<const void*, int> compImages; /**> Store a pointer to a resource and how many times his resource is used on the map.*/
                    std::vector<std::vector<EntityId>> visibleEntities;
                    RenderComponentManager* frcm; /**> The component manager.*/
                    float diagSize;
                    int cellWidth, cellHeight, cellDepth;
            };
        }
    }
}
#endif
