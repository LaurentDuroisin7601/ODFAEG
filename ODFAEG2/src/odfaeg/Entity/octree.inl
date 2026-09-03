namespace odfaeg {
    namespace entity {        
        template <typename Object>
        Octree<Object>::Octree(physic::BoundingBox volume, unsigned int maxObjectsPerNode) {
            Node rootNode;
            rootNode.id = 0;
            rootNode.volume = volume;
            rootNode.leaf = true;
            this->maxObjectsPerNode = maxObjectsPerNode;
            nodes.push_back(rootNode);       
        }
        template <typename Object>
        void Octree<Object>::addObject(Object object, physic::BoundingBox objectVolume) {                              
            //std::cout<<"add object"<<std::endl;
            
            
            
            insert(0, object, objectVolume);
            /*for (unsigned int i = 0; i < nodes.size(); i++) {
                if (nodes[i].leaf && nodes[i].objects.size() > 0) {
                std::cout<<"leaf ? "<<nodes[i].leaf<<","<<maxObjectsPerNode<<","<<nodes[i].objects.size()<<" "<<nodes[i].objectVolumes.size()<<std::endl;
                //system("PAUSE");
                }
            }*/
            //std::cout<<"ok 2"<<std::endl;            
        }
        template <typename Object>
        void Octree<Object>::insert(size_t id, Object object, physic::BoundingBox objectVolume) {
            Node& node = nodes[id];

            if (node.volume.intersects(objectVolume)) {
                    
                    
                if (!node.leaf) {
                    for (unsigned int j = 0; j < node.children.size(); j++) {
                        Node& child = nodes[node.children[j]];
                        if (!contains(child, object) && child.volume.intersects(objectVolume)) {
                            //std::cout<<"insert object : "<<j<<std::endl;
                            insert(node.children[j], object, objectVolume);
                            return;
                        }
                    }
                } 
                node.objects.push_back(object);
                node.objectVolumes.push_back(objectVolume);
                //std::cout<<"build octree"<<std::endl;
                if (node.objects.size() > maxObjectsPerNode) {
                    node.leaf = false;

                    auto volumes = node.volume.subdiv();
                    /*std::cout<<"child volume : "<<node.volume.getPosition()<<","<<node.volume.getSize()<<std::endl;
                    system("PAUSE");*/
                    // créer les enfants
                    //std::cout<<"créer enfants"<<std::endl;
                    for (unsigned int v = 0; v < 8; v++) {
                        Node child;
                        child.id = compteur++;
                        child.leaf = true;
                        child.parent = id;
                        child.volume = volumes[v];

                        nodes.push_back(child);
                        node.children.push_back(nodes.size() - 1);
                    }
                    //std::cout<<"enfants ok"<<std::endl;

                    // redistribuer les objets
                    for (unsigned int j = 0; j < node.objects.size(); j++) {
                        for (unsigned int k = 0; k < node.children.size(); k++) {

                            Node& child = nodes[node.children[k]];
                            /*std::cout<<"child volume : "<<child.volume.getPosition()<<","<<child.volume.getSize()<<std::endl;
                            std::cout<<"volume : "<<node.objectVolumes[j].getPosition()<<","<<node.objectVolumes[j].getSize()<<std::endl;*/

                            if (child.volume.intersects(node.objectVolumes[j])) {
                                //std::cout<<"intersects!"<<std::endl;
                                child.objects.push_back(node.objects[j]);
                                child.objectVolumes.push_back(node.objectVolumes[j]);
                                //break;
                            }
                        }
                    }
                    //std::cout<<"ok"<<std::endl;
                    // vider le node
                    node.objects.clear();
                    node.objectVolumes.clear();
                } 
            }
        }
        template <typename Object>
        void Octree<Object>::build(size_t id) {
            Node& node = nodes[id];

            if (node.objects.size() > maxObjectsPerNode) {

                node.leaf = false;

                auto volumes = node.volume.subdiv();

                // créer les enfants
                for (unsigned int v = 0; v < 8; v++) {
                    Node child;
                    child.id = compteur++;
                    child.leaf = true;
                    child.parent = id;
                    child.volume = volumes[v];

                    nodes.push_back(child);
                    node.children.push_back(nodes.size() - 1);
                }

                // redistribuer les objets
                for (unsigned int j = 0; j < node.objects.size(); j++) {
                    for (unsigned int i = 0; i < node.children.size(); i++) {

                        Node& child = nodes[node.children[i]];

                        if (child.volume.intersects(node.objectVolumes[j])) {
                            child.objects.push_back(node.objects[j]);
                            child.objectVolumes.push_back(node.objectVolumes[j]);
                        }
                    }
                }

                // vider le node
                node.objects.clear();
                node.objectVolumes.clear();

                // subdiviser les enfants qui dépassent
                for (unsigned int i = 0; i < node.children.size(); i++) {
                    Node& child = nodes[node.children[i]];
                    if (child.objects.size() > maxObjectsPerNode) {
                        build(node.children[i]);
                                            
                    }
                }
            }                  
        }
        template <typename Object>      
        void Octree<Object>::removeObject(Object object, physic::BoundingBox objectVolume) {
            for (unsigned int i = 0; i < nodes.size(); i++) {
                if (nodes[i].leaf && nodes[i].contains(object) && nodes[i].volume.intersects(objectVolume)) {
                    {
                        typename std::vector<Object>::iterator it;
                        std::vector<physic::BoundingBox>::iterator it2;
                        for (it = nodes[i].objects.begin(), it2 = nodes[i].objectVolumes.begin(); it != nodes[i].objects.end();) {
                            if (*it == &object) {
                                it = nodes[i].objects.erase(it);
                                it2 = nodes[i].objectVolumes.erase(it2); 
                                freeNodes(nodes[i]);   
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
        std::vector<Object> Octree<Object>::getObjects(physic::BoundingBox volume) {
            std::vector<Object> objects;
            Node node = nodes[0];
            if (node.volume.intersects(volume)) {
                getObjects(objects, node, volume);
            }
            return objects;
        }        
        template <typename Object>
        void Octree<Object>::getObjects(std::vector<Object>& objects, Node node, physic::BoundingBox volume) {
            for (unsigned int i = 0; i < node.objects.size(); i++) {
                objects.push_back(node.objects[i]);                
            }
            for (unsigned int c = 0; c < node.children.size(); c++) {
                if (nodes[node.children[c]].volume.intersects(volume)) {
                    getObjects(objects, nodes[node.children[c]], volume);
                }
            }
        }
        template <typename Object>
        std::vector<physic::BoundingBox> Octree<Object>::getObjectVolumes(physic::BoundingBox volume) {
            std::vector<physic::BoundingBox> objects;
            Node node = nodes[0];
            if (node.volume.intersects(volume)) {
                getObjectVolumes(objects, node, volume);
            }
            return objects;
        }        
        template <typename Object>
        void Octree<Object>::getObjectVolumes(std::vector<physic::BoundingBox>& objects, Node node, physic::BoundingBox volume) {
            for (unsigned int i = 0; i < node.objects.size(); i++) {
                objects.push_back(node.objectVolumes[i]);                
            }
            for (unsigned int c = 0; c < node.children.size(); c++) {
                if (nodes[node.children[c]].volume.intersects(volume)) {
                    getObjectVolumes(objects, nodes[node.children[c]], volume);
                }
            }
        }
        template <typename Object>
        void Octree<Object>::freeNodes(Node& parent) {
            bool empty = true;
            for (unsigned int i = 0; i < parent.children.size(); i++) {
                if (!nodes[parent.children[i]].objets.size() == 0) {
                    empty = false;
                    break;
                }
            }
            if (empty) {
                freeNodes(nodes[parent.parent]);
                typename std::vector<Node>::iterator it;
                for (unsigned int i = 0; i < parent.children.size(); i++) {
                    for (it = nodes.begin(); it != nodes.end;) {
                        if (*it == &nodes[parent.children[i]]) {
                            it = nodes.erase(it);
                        } else {
                            it++;
                        }  
                    }              
                }                
                for (it = nodes.begin(); it != nodes.end;) {
                    if (*it == &parent) {
                        it = nodes.erase(it);
                    } else {
                        it++;
                    }
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
        template <typename Object>
        std::deque<typename Octree<Object>::Node> Octree<Object>::getNodes() {
            return nodes;
        }
        template <typename Object>
        bool Octree<Object>::contains(Node& node, Object object) {
            for (unsigned int i = 0; i < node.objects.size(); i++) {
                if(object == node.objects[i])
                    return true;
            }            
            return false;
        }
        template <typename Object>
        bool Octree<Object>::empty() {
            for (unsigned int i = 0; i < nodes.size(); i++) {
                if (nodes[i].objects.size() != 0)
                    return false;
            }
            return true;
        }
    }
}