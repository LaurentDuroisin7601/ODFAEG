#ifndef ODFAEG_GRIDCELL_HPP
#define ODFAEG_GRIDCELL_HPP
#include "octree.hpp"
namespace odfaeg {
    namespace entity {
        template <typename Object>
        class GridCell {
            public :
                GridCell (physic::BoundingBox volume, math::Vec3f coords);
                math::Vec3f getCoords ();
                void addEntity (Object entity, physic::BoundingBox objectVolume);
                physic::BoundingBox getCellVolume ();
                bool isEntityInside ();
                vector<Object> getEntitiesInside ();                
                vector<Object> getEntitiesInside (std::string type);
                void removeEntity (Object object);                
                void deleteEntity (Object entity); 
                math::Vec3f getCenter ();
                bool isTraveled ();
                void setTraveled (bool traveled);
                Object getEntityInside (unsigned int index);
                unsigned int getNbEntitiesInside();                
                bool GridCell<Object>::containsEntity (Object entity);
                bool GridCell<Object>::isPassable ();
                void GridCell<Object>::setPassable (bool passable);
                void GridCell<Object>::setStateChanged (bool b);
                bool GridCell<Object>::isStateChanged ();
                bool GridCell<Object>::operator== (const GridCell &cellMap);
            private :
                bool passable, traveled, stateChanged;
                math::Vec3f coords;
                physic::BoundingVolume volume;
                Octree octree;
        };
    }
}
#endif