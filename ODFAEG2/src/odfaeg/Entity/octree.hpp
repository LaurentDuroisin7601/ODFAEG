namespace odfaeg {
    namespace entity {
        template <typename Object>
        class Octree {
            struct Node {
                physic::BoundingBox volume;
                std::vector<Object> objects;
                std::vector<physic::BoundingBox> objectVolumes;
                std::vector<unsigned int> children;
            }
            public :
            Octree(physic::BoundingBox volume, unsigned int maxObjectsPerNodes);
            void addObject(Object object, physic::BoundingBox objectVolume);
            void removeObject(Object object, physic::BoundingBox objectVolume);
            std::vector<Object> getObjects(physics::BoundingBox volume);
            void update(Object object);
            private :
            void getObjects(std::vector<Object>& objects, Node node, physics::BoundingBox volume);
            void build(Node& node);
            std::vector<Node> nodes;      
        };        
    }
}