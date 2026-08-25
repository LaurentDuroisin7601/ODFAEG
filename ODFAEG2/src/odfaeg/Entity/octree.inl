namespace ofdaeg {
    namespace entity {        
        template <typename Object>
        Octree<Object>::Octree(physic::BoundingBox volume, unsigned int maxObjectsPerNodes) {
            Node rootNode;
            rootNode.volume = volume;
            nodes.push_back(volume);       
        }
        template <typename Object>
        void Octree<Object>::addObject(Object object, physic::BoundingBox objectVolume) {                              
            for (unsigned int i = 0; i < nodes.size(); i++) {
                if (nodes[i].leaf && !contains(object) && nodes[i].volume.intersects(objectVolume)) {
                    nodes[i].objects.push_back(object);
                }               
            }
            build(nodes[0]);            
        }
        void template <typename Object>
        void Octree<Object>::build(Node& node) {
            if (node.objects.size() > maxObjectsPerNode) {
                node.leaf = false;    
                physic::BoundingBox volume = node.volume;
                std::array<physics::BoundingBox, 8> volumes = volume.subDivide();                
                for (unsigned int v = 0; v < volumes.size(); v++) {
                    Node child;
                    child.id = compteur.next();
                    child.volume.push_back(volumes[v]);                        
                    node.children.push_back(child.id);
                    nodes.push_back(child);
                }                
                for (unsigned int i = 0; i < node.objets.size(); i++) {
                    for (unsigned int j = 0; j < node.children.size(); j++) {
                        if (nodes[children[j]].volume.intersects(node.objectsVolumes[i])) {
                            nodes[children[j]].objects.push_back(node.objects[i]);
                            nodes[children[j]].objectsVolumes.push_back(node.objectsVolumes[i]);
                            build(nodes[children[j]]);
                        }
                    }
                }
                node.objects.clear(); 
                node.objectsVolumes.clear();                            
            }
        }  
        void template <typename Object>      
        void Octree<Object>::removeObject(Object object, physic::BoundingBox objectVolume) {
            for (unsigned int i = 0; i < nodes.size(); i++) {
                if (nodes[i].leaf && nodes[i].contains(object) && nodes[i].volume.intersects(objectVolume)) {
                    {
                        std::vector<Object>::iterator it;
                        std::vector<physics::BoundingBox>::iterator it2;
                        for (it = nodes[i].objects.begin(), it2 = nodes[i].objectVolumes.begin(); it != nodes[i].objects.end();) {
                            if (*it == &object) {
                                it = nodes[i].objects.erase(it);
                                it2 = nodes[i].objectVolumes.erase(it2);    
                            }
                        }
                    }
                }               
            }
        }
        template <typename Object>
        void Octree<Object>::update(Object object) {
            removeObject(object);
            addObject(object);
        }
        template <typename Object>
        std::vector<Object> Octree<Object>::getObjects(physics::BoundingBox volume) {
            Node node = nodes[0];
            if (node.volume.intersects(volume)) {
                getObjects(objects, node, volume)
            }
            return objects;
        }        
        template <typename Object>
        void Octree<Object>::getObjects(std::vector<Object>& objects, Node node, physics::BoundingBox volume) {
            for (unsigned int i = 0; < node.objects.size(); i++) {
                objects.push_back(node.objects[i]);                
            }
            for (unsigned int c = 0; c < node.children.size(); c++) {
                if (nodes[node.children[c]].volume.intersects(volume)) {
                    getObjects(objects, nodes[node.children[c]], volume);
                }
            }
        }
        template <typename Object>
        bool Octree<Object>::contains(Object object) {
            for (unsigned int i = 0; i < nodes.size(); i++) {
                for (unsigned int j = 0; j < nodes[i].objects.size(); j++) {
                    if(object == nodes[i].objects[j])
                        return true;
                }
            }
            return false;
        }
    }
}