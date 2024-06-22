#include "../../../../include/odfaeg/Graphics/ECS/cell.hpp"
using namespace std;
namespace odfaeg {
    namespace graphic {
        namespace ecs {
            Cell::Cell (physic::BoundingPolyhedron bp, math::Vec3f coords) {
                cellVolume = bp;
                passable = true;
                stateChanged = false;
                traveled = false;
                this->coords = coords;
            }

            math::Vec3f Cell::getCoords () {
                return coords;
            }

            void Cell::addEntity (EntityId entity) {
                if (!containsEntity(entity)) {
                    entitiesInside.push_back(entity);
                }
            }
            physic::BoundingPolyhedron Cell::getCellVolume () {
                return cellVolume;
            }

            bool Cell::isEntityInside () {
                if (entitiesInside.size() != 0)
                        return true;
                return false;
            }

            vector<EntityId> Cell::getEntitiesInside () {
                return entitiesInside;
            }

            bool Cell::removeEntity (EntityId entity) {
                typename vector<EntityId>::iterator it;
                for (it = entitiesInside.begin(); it != entitiesInside.end();) {
                    if (entity == *it) {
                        //std::cout<<"remove entity : "<<it->get()<<std::endl;
                        //std::cout<<"old size : "<<entitiesInside.size()<<std::endl;
                        it = entitiesInside.erase(it);
                        /*for (unsigned int i = 0; i < entitiesInside.size(); i++) {
                            std::cout<<"entity inside : "<<entitiesInside[i].get()<<std::endl;
                        }*/
                        //std::cout<<"new size : "<<entitiesInside.size()<<std::endl;
                        return true;
                    } else
                        it++;

                }
                return false;
            }
            /*bool Cell::removeEntity (std::string type) {
                typename vector<std::unique_ptr<Entity>>::iterator it;
                for (it = entitiesInside.begin(); it != entitiesInside.end();) {
                    if (it->get()->getType() == type) {
                        it->release();
                        it = entitiesInside.erase(it);
                        return true;
                    } else
                        it++;
                }
                return false;
            }*/
            math::Vec3f Cell::getCenter () {
                return cellVolume.getCenter();
            }

            bool Cell::isTraveled () {
                return traveled;
            }

            void Cell::setTraveled (bool traveled) {
                this->traveled = traveled;
            }

            EntityId Cell::getEntityInside (unsigned int index) {
                if (index >= 0 && index < entitiesInside.size()) {
                    EntityId entity = entitiesInside[index];
                    return entity;
                }
            }
            unsigned int Cell::getNbEntitiesInside() {
                return entitiesInside.size();
            }
            bool Cell::containsEntity (EntityId entity) {
                for (unsigned int i = 0; i < entitiesInside.size(); i++) {
                    if (entitiesInside[i] == entity) {
                        return true;
                    }
                }
                return false;
            }

            bool Cell::isPassable () {
                return passable;
            }

            void Cell::setPassable (bool passable) {
                this->passable = passable;
            }

            void Cell::setStateChanged (bool b) {
                this->stateChanged = b;
            }

            bool Cell::isStateChanged () {
                return stateChanged;
            }

            bool Cell::operator== (const Cell &Cell) {
                return cellVolume == cellVolume;
            }
            bool Cell::removeEntity (std::string type) {
                typename vector<EntityId>::iterator it;
                for (it = entitiesInside.begin(); it != entitiesInside.end();) {
                    if (getComponent<EntityInfoComponent>(*it)->groupName == type) {
                        it = entitiesInside.erase(it);
                        return true;
                    } else
                        it++;
                }
                return false;
            }
        }
    }
}
