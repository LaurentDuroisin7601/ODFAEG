#ifndef CELL_MAP
#define CELL_MAP
#include "../../BoundingAreas/boundingPolygon.h"
#include "../../Graphics/2D/entity.h"

namespace odfaeg {
    namespace graphic {
        class CellMap {
            public :
                CellMap (BoundingPolygon *bp, Vec2f coords, int nbLayers);
                BoundingPolygon* getPolygon ();
                void setPolygon (BoundingPolygon *bp);
                Entity* getEntityInside (int index, int l);
                std::vector<std::vector<Entity*> > getEntitiesInside ();
                bool isEntityInside ();
                bool containsEntity (Entity *entity, int l);
                void addEntity (Entity *entity, int l);
                bool removeEntity (Entity * entity, int l);

                void setStateChanged (bool b);
                bool isStateChanged ();
                bool isPassable ();
                void setPassable (bool passable);
                bool operator== (const CellMap &cell) const;
                Vec3f getCoords ();
                Vec3f& getCenter ();
                bool isTraveled ();
                void setTraveled (bool traveled);
                ~CellMap();
            private :
                std::vector<std::vector<Entity*> > entityInside;
                BoundingPolygon *poly;
                bool passable, stateChanged, traveled;
                Vec3f coords, center;
        };
    }
}
#endif
