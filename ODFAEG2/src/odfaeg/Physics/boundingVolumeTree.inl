namespace ofdaeg {
    namespace phycic {
        enum Type {
            OCTREE, BSP
        };
        template <typename Volume, typename Object>
        BoundingVolumeTree<Volume, Object>::BoundingVolumeTree(unsigned int maxObjectsPerNodes) {
            
        }
        template <typename Volume, typename Object>
        void BoundingVolumeTree<Volume, Object>::addObject(Object object) {
            bool needToRebuild = false;
            math::Vec3f treeSize = globalVolume.getSize();
            if (object.getPosition().x() + object.getSize().x > treeSize.x()) {
                treeSize.x() = object.getPosition().x() + object.getSize().x();
                needToRebuild = true;
            }
            if (object.getPosition().y() + object.getSize().y > treeSize.y()) {
                treeSize.y() = object.getPosition().y() + object.getSize().y();
                needToRebuild = true;
            }
            if (object.getPosition().z() + object.getSize().z > treeSize.z()) {
                treeSize.z() = object.getPosition().z() + object.getSize().z();
                needtoRebuild = true;
            } 
            Node rootNode = (nodes.size() == 0) ? Node() : nodes[0];
            if (needToRebuild) {
                globalVolume.setSize(treesSize);
                rootNode.volume = globalVolume;                 
            }            
            if (nodes.size() == 0) {  
                rootNode.leaf = true; 
                rootNode.id = compteur.next(); 
                nodes.push_back(rootNode);
            }                          
            for (unsigned int i = 0; i < nodes.size(); i++) {
                if (nodes[i].leaf && !nodes[i].contains(object) && nodes[i].volume.intersects(object)) {
                    object.nodeId = nodes[i].id;
                    nodes[i].objects.push_back(object);
                }               
            }
            build<Object>(nodes[0]);            
        }
        void template <typename Volume, typename Object>
        void BoundingVolumeTree<Volume, Object>::build(Node& node) {
            if (node.objects.size() > maxObjectsPerNode) {
                node.leaf = false;    
                Volume volume = node.volume;
                std::vector<Volume> volumes;
                if (BSP) {
                    volumes = volume.subDivide(2);  
                                  
                } else {
                    volumes = volume.subDivide(8); 
                }
                for (unsigned int v = 0; v < volumes.size(); v++) {                
                    for (unsigned int i = 0; i < node.objects.size(); i++) {
                        if (volumes[v].intersects(node.objects[i])) {
                            nodes.push_back(Node());
                            Node& child = nodes.back();
                            child.id = compteur.next();
                            child.volume = volumes[v];
                            child.parent = &node;
                            objects[i].nodeId = child.id;
                            node.objects.push_back(node.objects[i]);
                            node.children.push_back(&child);
                            build(child);                                
                        }
                    }
                }  
                node.objects.clear();                             
            }
        }
        template <typename Volume, typename Object>
        void BoundingVolumeTree<Volume, Object>::removeObject(Object object) {
            for (unsigned int i = 0; i < nodes.size(); i++) {
                if (nodes[i].leaf && nodes[i].volume.intersects(object)) {
                    nodes[i].objects.clear();
                    Node& parent = *nodes[i].parent;
                    parent.removeChild(nodes[i]);
                    std::vector<Node>::iterator it;
                    for (it = nodes.begin(); it != nodes.end(); it++) {
                        if (*it == &nodes[i]) {
                            nodes.erase(it);
                        }
                    }
                }               
            }
        }
        template <typename Volume, typename Object>
        void BoundingVolumeTree<Volume, Object>::update(Object object) {
            removeObject(object);
            addObject(object);
        }
        template <typename Volume, typename Object>        
        void BoundingVolumeTree<Volume>::update(Object object) {
            removeObject(object);
            addObject(object);
        }
        template <typename Volume, typename Object>
        std::vector<Object> BoundingVolumeTree<Volume, Object>::getObjects(Volume volume) {
            std::vector<Object> objects;
            if (nodes.size() == 0)
                return objects;
            Node node = nodes[0];
            if (node.volume.intersects(volume)) {
                getObjects(objects, node, volume)
            }
            return objects;
        }
        template <typename Volume, typename Object>
        void BoundingVolumeTree<Volume, Object>::getObjects(std::vector<Object>& objects, Node node, Volume volume) {
            for (unsigned int i = 0; < node.objects.size(); i++) {
                objects.push_back(node.objects[i]);                
            }
            for (unsigned int c = 0; c < node.children.size(); c++) {
                if (node.children[c].volume.intersects(volume)) {
                    getObjects(objects, *node.children[c], volume);
                }
            }
        }
    }
}