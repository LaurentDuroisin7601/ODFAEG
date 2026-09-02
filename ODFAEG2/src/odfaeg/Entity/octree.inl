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
            for (unsigned int i = 0; i < nodes.size(); i++) {
                
                if (nodes[i].leaf && !contains(nodes[i], object) && nodes[i].volume.intersects(objectVolume)) {
                    /*std::cout<<"volume : "<<nodes[0].volume.getPosition()<<","<<nodes[0].volume.getSize()<<std::endl;
                    std::cout<<"object volume : "<<objectVolume.getPosition()<<","<<objectVolume.getSize()<<std::endl;*/
                    nodes[i].objects.push_back(object);
                    nodes[i].objectVolumes.push_back(objectVolume);
                }               
            }
            //std::cout<<"ok 1"<<std::endl;
            build(nodes[0]);
            //std::cout<<"ok 2"<<std::endl;            
        }
        template <typename Object>
        void Octree<Object>::build(Node& node) {
            if (node.objects.size() > maxObjectsPerNode) {
                //std::cout<<"subdiv"<<std::endl;
                node.leaf = false;    
                physic::BoundingBox volume = node.volume;
                //std::cout<<"parent volume : "<<volume.getPosition()<<","<<volume.getSize()<<std::endl;
                std::array<physic::BoundingBox, 8> volumes = volume.subdiv();    
                //std::cout<<"subddiv ok : "<<std::endl;
                for (unsigned int v = 0; v < volumes.size(); v++) {
                    Node child;
                    child.id = compteur;
                    child.leaf = true;
                    compteur++;
                    
                    child.parent = node.id;
                    child.volume = volumes[v]; 
                    
                    nodes.push_back(child);
                    //std::cout<<"add child : "<<nodes.size()-1<<std::endl;
                    node.children.push_back(nodes.size()-1);
                }    
                std::cout<<"subdivided"<<std::endl;            
                for (unsigned int i = 0; i < node.children.size(); i++) {
                    for (unsigned int j = 0; j < node.objects.size(); j++) {
                        //std::cout<<"size : "<<nodes.size()<<"child : "<<node.children[j]<<std::endl;
                       /* std::cout<<"node volume : "<<nodes[node.children[j]].volume.getPosition()<<","<<nodes[node.children[j]].volume.getSize()<<std::endl;
                        std::cout<<"object volume : "<<node.objectVolumes[i].getPosition()<<","<<node.objectVolumes[i].getSize()<<std::endl;*/
                        if (nodes[node.children[i]].volume.intersects(node.objectVolumes[j])) {
                            //std::cout<<"rebuild : "<<node.objects.size()<<std::endl;
                            nodes[node.children[i]].objects.push_back(node.objects[j]);
                            nodes[node.children[i]].objectVolumes.push_back(node.objectVolumes[j]);
                            
                                                       
                        }                                               
                    }
                    //build(nodes[node.children[i]]);
                }
                std::cout<<"objects added"<<std::endl;
                for (unsigned int i = 0; i < node.children.size(); i++) {
                    Node& child = nodes[node.children[i]];
                    //std::cout<<"rebuild"<<std::endl;
                    if (child.objects.size() > maxObjectsPerNode) {
                        
                        build(child);
                    }
                }
                node.objects.clear(); 
                node.objectVolumes.clear();                            
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