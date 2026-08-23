namespace ofdaeg {
    namespace phycic {
        enum Type {
            OCTREE, BSP
        };
        template <typename Volume>
        BoundingVolumeTree<Volume>::BoundingVolumeTree() {
            
        }
        template <typename Volume, typename Object>
        void BoundingVolumeTree::addObject(Object object) {
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
        void BoundingVolumeTree<Volume>::build(Node& node) {
            if (node.objects.size() > 1) {
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
        void BoundingVolumeTree<Volume>::removeObject(Object object) {
            for (unsigned int i = 0; i < nodes.size(); i++) {
                if (nodes[i].leaf && nodes[i].volume.intersects(object)) {
                    nodes[i].objects.clear();
                    Node& parent = nodes[i].parent;
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
        void BoundingVolumeTree<Volume>::updateObject(Object object) {
            removeObject(object);
            addObject(object);
        }
    }
}