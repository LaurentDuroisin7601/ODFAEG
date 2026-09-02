#ifndef ODFAEG_OCTREE_HPP
#define ODFAEG_OCTREE_HPP
#include "../Core/compiletime_counter.hpp"
#include "../Physics/boundingBox.hpp"
#include <deque>
namespace odfaeg {
    namespace entity {
        template <typename Object>
        class Octree {
            public :
            struct Node {
                unsigned int id;
                physic::BoundingBox volume;
                std::vector<Object> objects;
                std::vector<physic::BoundingBox> objectVolumes;
                unsigned int parent;
                std::vector<unsigned int> children;
                bool leaf;
            };           
            Octree(physic::BoundingBox volume, unsigned int maxObjectsPerNodes);
            void addObject(Object object, physic::BoundingBox objectVolume);
            void removeObject(Object object, physic::BoundingBox objectVolume);
            std::vector<Object> getObjects(physic::BoundingBox volume);
            std::vector<physic::BoundingBox> getObjectVolumes(physic::BoundingBox volume);
            void update(Object object);
            bool contains(Object object);
            bool contains(Node& node, Object object);
            bool empty();
            std::deque<Node> getNodes();
            private :
            unsigned int maxObjectsPerNode;
            void getObjectVolumes(std::vector<physic::BoundingBox>& objects, Node node, physic::BoundingBox volume);
            void getObjects(std::vector<Object>& objects, Node node, physic::BoundingBox volume);
            void build(Node& node);
            void freeNodes(Node& node);
            std::deque<Node> nodes;
            inline static unsigned int compteur = 0;

        };        
    }
}
#include "octree.inl"
#endif