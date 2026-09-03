namespace odfaeg {
    namespace entity {
        using namespace std;
         template <typename Object>
        GridCell<Object>::GridCell () : octree(physic::BoundingBox(0, 0, 0, 0, 0, 0), 8) {
            passable = true;
            stateChanged = false;
            traveled = false;

        }
        template <typename Object>
        GridCell<Object>::GridCell (physic::BoundingBox volume, math::Vec3f coords) : octree(volume, 8) {
            this->volume = volume;
            passable = true;
            stateChanged = false;
            traveled = false;
            this->coords = coords;
        }
        template <typename Object>
        math::Vec3f GridCell<Object>::getCoords () {
            return coords;
        }
        template <typename Object>
        void GridCell<Object>::addEntity (Object entity, physic::BoundingBox objectVolume) {
            if (!containsEntity(entity)) {                
                octree.addObject(entity, objectVolume);
            }
        }
        template <typename Object>
        physic::BoundingBox GridCell<Object>::getCellVolume () {
            return volume;
        }
        template <typename Object>
        bool GridCell<Object>::isEntityInside () {
            if (octree.getObjects(volume).size() != 0)
                    return true;
            return false;
        }
        template <typename Object>
        std::deque<typename Octree<Object>::Node>& GridCell<Object>::getOctreeNodes() {
            return octree.getNodes();
        }
        template <typename Object>
        bool GridCell<Object>::empty() {
            return octree.empty();
        }
        template <typename Object>
        vector<Object> GridCell<Object>::getEntitiesInside () {
            vector<Object> entitiesInside;
            for (unsigned int i = 0; i < octree.getObjects(volume).size(); i++)
                entitiesInside.push_back(octree.getObjects(volume)[i]);
            return entitiesInside;
        }
        template <typename Object>
        vector<Object> GridCell<Object>::getEntitiesInside (std::string type) {
            vector<Object> entitiesInside;
            for (unsigned int i = 0; i < octree.getObjects(volume).size(); i++)
                if (octree.getObjects(volume)[i]->getType() == type)
                    entitiesInside.push_back(octree.getObjects(volume)[i]);
            return entitiesInside;
        }
        template <typename Object>
        void GridCell<Object>::removeEntity (Object object) {
           octree.removeObject(object);
        }
        template <typename Object>
        void GridCell<Object>::deleteEntity (Object entity) {
            for (unsigned int i = 0; i < octree.getObjects(volume).size(); i++) {
                if (octree.getObjects(volume)[i] == entity) {
                    removeEntity(entity);
                    delete entity;
                }
            }
        }
        template <typename Object>
        math::Vec3f GridCell<Object>::getCenter () {
            return volume.getCenter();
        }
        template <typename Object>
        bool GridCell<Object>::isTraveled () {
            return traveled;
        }
        template <typename Object>
        void GridCell<Object>::setTraveled (bool traveled) {
            this->traveled = traveled;
        }
        template<typename Object>
        Object GridCell<Object>::getEntityInside (unsigned int index, physic::BoundingBox& bx) {
            if (index >= 0 && index < octree.getObjects(volume).size()) {
                Object entity = octree.getObjects(volume)[index];
                bx = octree.getObjectVolumes(volume)[index];
                return entity;
            }
            return nullptr;
        }
        template <typename Object>
        unsigned int GridCell<Object>::getNbEntitiesInside() {
            return octree.getObjects(volume).size();
        }
        template <typename Object>
        bool GridCell<Object>::containsEntity (Object entity) {            
            return octree.contains(entity);
        }
        template <typename Object>
        bool GridCell<Object>::isPassable () {
            return passable;
        }
        template <typename Object>
        void GridCell<Object>::setPassable (bool passable) {
            this->passable = passable;
        }
        template <typename Object>
        void GridCell<Object>::setStateChanged (bool b) {
            this->stateChanged = b;
        }
        template <typename Object>
        bool GridCell<Object>::isStateChanged () {
            return stateChanged;
        }
        template <typename Object>
        bool GridCell<Object>::operator== (const GridCell &gridCell) const {
            return &volume == &gridCell.volume;
        }
    }
}