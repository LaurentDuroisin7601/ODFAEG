#ifndef ODFAEG_BSPTREE_HPP
#define ODFAEG_BSPTREE_HPP
#include "../Physics/boundingBox.hpp"
#include "../Math/plane.hpp"
#include <deque>
namespace odfaeg {
    namespace entity  {
        template <typename Object>
        class BSPTree {
            public :
            struct Node {
                unsigned int id;
                math::Plane plane;
                std::vector<Object> objects;
                std::vector<physic::BoundingBox> objectVolumes;
                unsigned int parent;
                unsigned int leftChild;
                unsigned int rightChild;
                bool leaf;
            };           
            BSPTree(physic::BoundingBox volume, unsigned int maxObjectsPerNodes);
            void addObject(Object object, physic::BoundingBox objectVolume);
            void removeObject(Object object, physic::BoundingBox objectVolume);
            std::vector<Object> getObjects(physic::BoundingBox volume);
            std::vector<physic::BoundingBox> getObjectVolumes(physic::BoundingBox volume);
            void update(Object object);
            bool contains(Object object);
            bool contains(Node& node, Object object);
            bool empty();
            std::deque<Node>& getNodes();
            private :
            void subdivide(size_t id);
            unsigned int maxObjectsPerNode;
            void getObjectVolumes(std::vector<physic::BoundingBox>& objects, Node node, physic::BoundingBox volume);
            void getObjects(std::vector<Object>& objects, Node node, physic::BoundingBox volume);
            void insert(size_t id, Object object, physic::BoundingBox objectVolume, unsigned int depth=0);
            void freeNodes(Node& node);
            std::deque<Node> nodes;
            inline static unsigned int compteur = 0;            
        };     
    }
}
#include "bspTree.inl"
#endif