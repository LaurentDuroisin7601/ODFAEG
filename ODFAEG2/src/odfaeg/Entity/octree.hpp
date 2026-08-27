#ifndef ODFAEG_OCTREE_HPP
#define ODFAEG_OCTREE_HPP
#include "../Core/compiletime_counter.hpp"
#include "../Physics/boundingBox.hpp"
namespace odfaeg {
    namespace entity {
        template <typename Object>
        class Octree {
            struct Node {
                unsigned int id;
                physic::BoundingBox volume;
                std::vector<Object> objects;
                std::vector<physic::BoundingBox> objectVolumes;
                unsigned int parent;
                std::vector<unsigned int> children;
                bool leaf;
            };
            public :
            Octree(physic::BoundingBox volume, unsigned int maxObjectsPerNodes);
            void addObject(Object object, physic::BoundingBox objectVolume);
            void removeObject(Object object, physic::BoundingBox objectVolume);
            std::vector<Object> getObjects(physic::BoundingBox volume);
            void update(Object object);
            bool contains(Object object);
            private :
            unsigned int maxObjectsPerNode;
            void getObjects(std::vector<Object>& objects, Node node, physic::BoundingBox volume);
            void build(Node& node);
            void freeNodes(Node& node);
            std::vector<Node> nodes;
            core::Compteur<> compteur;      
        };        
    }
}
#include "octree.inl"
#endif