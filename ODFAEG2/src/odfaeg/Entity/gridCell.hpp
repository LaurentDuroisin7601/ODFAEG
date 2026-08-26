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
                std::vector<Object> getEntitiesInside ();                
                std::vector<Object> getEntitiesInside (std::string type);
                void removeEntity (Object object);                
                void deleteEntity (Object entity); 
                math::Vec3f getCenter ();
                bool isTraveled ();
                void setTraveled (bool traveled);
                Object getEntityInside (unsigned int index);
                unsigned int getNbEntitiesInside();                
                bool containsEntity (Object entity);
                bool isPassable ();
                void setPassable (bool passable);
                void setStateChanged (bool b);
                bool isStateChanged ();
                bool operator== (const GridCell &cellMap) const;
            private :
                bool passable, traveled, stateChanged;
                math::Vec3f coords;
                physic::BoundingBox volume;
                Octree<Object> octree;
        };
    }
}
#include "gridCell.inl"
#endif