namespace odfaeg {
    namespace entity {
        using namespace std;
        template <typename Object>
        GridCell<Object>::GridCell (physic::BoundingBox volume, math::Vec3f coords) {
            this->volume = volume;
            passable = true;
            stateChanged = false;
            traveled = false;
            this->coords = coords;
        }
        template <typename Object>
        math::Vec3f GridCell<Object>::GridCell::getCoords () {
            return coords;
        }
        template <typename Object>
        void GridCell<Object>::GridCell::addEntity (Object* entity) {
            if (!containsEntity(entity)) {
                std::unique_ptr<Object> ptr;
                ptr.reset(entity);
                octree.addObject(std::move(ptr));
            }
        }
        template <typename Object>
        physic::BoundingBox GridCell<Object>::GridCell::getCellVolume () {
            return cellVolume;
        }
        template <typename Object>
        bool GridCell<Object>::GridCell::isEntityInside () {
            if (entityInside.size() != 0)
                    return true;
            return false;
        }
        template <typename Object>
        vector<Object> GridCell<Object>::GridCell::getEntitiesInside () {
            vector<Object> entitiesInside;
            for (unsigned int i = 0; i < octree.getObjects(volume).size(); i++)
                entitiesInside.push_back(octree.getObjects(volume)[i]);
            return entitiesInside;
        }
        template <typename Object>
        vector<Object> GridCell<Object>::GridCell::getEntitiesInside (std::string type) {
            vector<Object> entitiesInside;
            for (unsigned int i = 0; i < octree.getObjects(volume).size(); i++)
                if (octree.getObjects(volume)[i]->getType() == type)
                    entitiesInside.push_back(entityInside[i].get());
            return entitiesInside;
        }
        template <typename Object>
        void GridCell<Object>::removeEntity (Object object) {
           octree.removeObject(object);
        }
        template <typename Object>
        void GridCell<Object>::deleteEntity (Object *entity) {
            for (unsigned int i = 0; i < octree.getObjects(volume).size(); i++) {
                if (octree.getObjects(volume)[i].get() == entity) {
                    removeEntity(entity);
                    delete entity;
                }
            }
        }
        template <typename Object>
        void GridCell<Object>::removeEntity (std::string type) {
            vector<std::unique_ptr<Object>>& entityInside = octree.getObjects(volume);
            typename vector<std::unique_ptr<Object>>::iterator it;
            for (it = entityInside.begin(); it != entityInside.end();) {
                if (it->get()->getType() == type) {
                    it->release();
                    it = entityInside.erase(it);
                    return true;
                } else
                    it++;
            }
            return false;
        }
        template <typename Object>
        void GridCell<Object>::deleteEntity (std::string type) {
            vector<std::unique_ptr<Object>>& entityInside = octree.getObjects(volume);
            typename vector<std::unique_ptr<Object>>::iterator it;
            for (it = entityInside.begin(); it != entityInside.end();) {
                if (it->get()->getType() == type) {
                    it = entityInside.erase(it);
                } else
                    it++;
            }
        }
        template <typename Object>
        math::Vec3f GridCell<Object>::getCenter () {
            return volume->getCenter();
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
        Object* CellMap<Object>::getEntityInside (unsigned int index) {
            if (index >= 0 && index < entityInside.size()) {
                Object* entity = octree.getObjects()[index].get();
                return entity;
            }
            return nullptr;
        }
        template <typename Object>
        unsigned int GridCell<Object>::getNbEntitiesInside() {
            return octree.getObjects(volume).size();
        }
        template <typename Object>
        bool GridCell<Object>::containsEntity (Entity *entity) {            
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
        bool GridCell<Object>::operator== (const CellMap &cellMap) const {
            return volume == volume;
        }
    }
}