namespace odfaeg {
    namespace entity {
        template <typename Object>
        BSPTree<Object>::BSPTree(physic::BoundingBox volume, unsigned int maxObjectsPerNode) {
            Node rootNode;
            rootNode.id = 0;
            rootNode.plane = math::Plane();
            rootNode.volume = volume;
            /*std::cout<<"volume : "<<volume.getSize()<<std::endl;
            system("PAUSE");*/
            rootNode.leaf = true;
            this->maxObjectsPerNode = maxObjectsPerNode;
            nodes.push_back(rootNode);       
        }
        template <typename Object>
        void BSPTree<Object>::addObject(Object object, physic::BoundingBox objectVolume) {                              
            //std::cout<<"add object"<<std::endl;
            
            
            
            insert(0, object, objectVolume);
            //std::cout<<"nb nodes : "<<nodes.size()<<std::endl;
            //system("PAUSE");
            /*for (unsigned int i = 0; i < nodes.size(); i++) {
                if (nodes[i].leaf && nodes[i].objects.size() > 0) {
                std::cout<<"leaf ? "<<nodes[i].leaf<<","<<maxObjectsPerNode<<","<<nodes[i].objects.size()<<" "<<nodes[i].objectVolumes.size()<<std::endl;
                //system("PAUSE");
                }
            }*/
            //std::cout<<"ok 2"<<std::endl;            
        }
        template <typename Object>
        void BSPTree<Object>::insert(size_t id, Object object, physic::BoundingBox objectVolume, unsigned int depth) {
            Node& node = nodes[id];
            /*std::cout<<"add : "<<node.volume.getPosition()<<","<<node.volume.getSize()<<std::endl;
            std::cout<<"object volume : "<<objectVolume.getPosition()<<objectVolume.getSize()<<std::endl;*/
            if (!node.leaf) {                    
                if (node.plane.whichSide(objectVolume.getCenter()) < 0) {
                    //std::cout<<"insert object : "<<j<<std::endl;
                    insert(node.leftChild, object, objectVolume);
                    //return;
                } else {
                    insert(node.rightChild, object, objectVolume);
                }     
            } else {
                
                //std::cout<<"insert object : "<<objectVolume.getPosition()<<","<<objectVolume.getSize()<<std::endl;
                //std::cout<<"insert meshlet : "<<std::endl;
                node.objects.push_back(object);
                node.objectVolumes.push_back(objectVolume);
                subdivide(id);                      
            }
            
        }
        template <typename Object>
        void BSPTree<Object>::subdivide(size_t id) {
            Node& node = nodes[id]; 
            if (node.objects.size() > maxObjectsPerNode) {
                //std::cout<<"subdivide"<<std::endl;
                node.leaf = false;
                std::vector<math::Vec3f> centers;
                for (unsigned int i = 0; i < node.objectVolumes.size(); i++) {
                    centers.push_back(node.objectVolumes[i].getCenter());                    
                }
                math::Vec3f mean = math::Computer::getMoy(centers);
                math::Matrix4f C = math::Computer::computeCovariance(centers);
                math::Vec3f v = math::Computer::principalEigenVector(C);
                node.plane = math::Plane(v, mean);
                Node leftChild, rightChild;
                leftChild.id = compteur++;
                rightChild.id = compteur++;
                leftChild.leaf = true;
                rightChild.leaf = true;
                leftChild.parent = id;
                rightChild.parent = id;
                leftChild.volume = node.volume;
                rightChild.volume = node.volume;
                nodes.push_back(leftChild);
                node.leftChild = nodes.size() - 1;
                nodes.push_back(rightChild);
                node.rightChild = nodes.size() - 1; 
                node.children.push_back(node.leftChild);
                node.children.push_back(node.rightChild); 
                // redistribuer les objets
                for (unsigned int j = 0; j < node.objects.size(); j++) {
                    if (node.plane.whichSide(node.objectVolumes[j].getCenter()) < 0) {
                        nodes[node.leftChild].objects.push_back(node.objects[j]);
                        nodes[node.leftChild].objectVolumes.push_back(node.objectVolumes[j]);
                    } else {
                        nodes[node.rightChild].objects.push_back(node.objects[j]);
                        nodes[node.rightChild].objectVolumes.push_back(node.objectVolumes[j]);
                    }
                }    
                node.objects.clear();
                node.objectVolumes.clear(); 
                for (unsigned int i = 0; i < node.children.size(); i++) {
                    subdivide(node.children[i]);
                }
            }
        }
        template <typename Object>      
        void BSPTree<Object>::removeObject(Object object, physic::BoundingBox objectVolume) {
            for (unsigned int i = 0; i < nodes.size(); i++) {
                if (nodes[i].leaf && nodes[i].contains(object)) {
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
        void BSPTree<Object>::update(Object object) {
            removeObject(object);
            addObject(object);
        }
        template <typename Object>
        std::vector<Object> BSPTree<Object>::getObjects(physic::BoundingBox volume) {
            std::vector<Object> objects;
            Node node = nodes[0];
            getObjects(objects, node, volume);            
            return objects;
        }        
        template <typename Object>
        void BSPTree<Object>::getObjects(std::vector<Object>& objects, Node node, physic::BoundingBox volume) {
            for (unsigned int i = 0; i < node.objects.size(); i++) {
                objects.push_back(node.objects[i]);                
            }
            for (unsigned int i = 0; i < node.children.size(); i++) {
                getObjects(objects, nodes[node.children[i]], volume);                
            } 
        }
        template <typename Object>
        std::vector<physic::BoundingBox> BSPTree<Object>::getObjectVolumes(physic::BoundingBox volume) {
            std::vector<physic::BoundingBox> objects;
            Node node = nodes[0];
            getObjectVolumes(objects, node, volume);            
            return objects;
        }        
        template <typename Object>
        void BSPTree<Object>::getObjectVolumes(std::vector<physic::BoundingBox>& objects, Node node, physic::BoundingBox volume) {
            for (unsigned int i = 0; i < node.objects.size(); i++) {
                objects.push_back(node.objectVolumes[i]);                
            }
            for (unsigned int i = 0; i < node.children.size(); i++) {
                getObjectVolumes(objects, nodes[node.children[i]], volume);                
            } 
        }
        template <typename Object>
        void BSPTree<Object>::freeNodes(Node& parent) {
            bool empty = true;
            if (!nodes[parent.leftChild].objets.size() == 0
                || !nodes[parent.rightChild].objets.size() == 0) {
                empty = false;
                return;
            }            
            if (empty) {  
                freeNodes(nodes[parent.parent]);              
                typename std::vector<Node>::iterator it; 
                for (unsigned int i = 0; i < parent.children.size(); i++) {               
                    for (it = nodes.begin(); it != nodes.end();) {                    
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
        bool BSPTree<Object>::contains(Object object) {
            for (unsigned int i = 0; i < nodes.size(); i++) {
                for (unsigned int j = 0; j < nodes[i].objects.size(); j++) {
                    if(object == nodes[i].objects[j])
                        return true;
                }  
            }          
            return false;
        }
        template <typename Object>
        std::deque<typename BSPTree<Object>::Node>& BSPTree<Object>::getNodes() {
            //std::cout<<"octree nb nodes : "<<nodes.size()<<std::endl;
            /*for (unsigned int i = 0; i < nodes.size(); i++) {
                if (nodes[i].leaf && nodes[i].objects.size() > 0) {
                std::cout<<"id : "<<nodes[i].id<<","<<maxObjectsPerNode<<","<<nodes[i].objects.size()<<" "<<nodes[i].objectVolumes.size()<<std::endl;
                //system("PAUSE");
                }
            }*/
            return nodes;
        }
        template <typename Object>
        bool BSPTree<Object>::contains(Node& node, Object object) {
            for (unsigned int i = 0; i < node.objects.size(); i++) {
                if(object == node.objects[i])
                    return true;
            }            
            return false;
        }
        template <typename Object>
        bool BSPTree<Object>::empty() {
            for (unsigned int i = 0; i < nodes.size(); i++) {
                if (nodes[i].objects.size() != 0)
                    return false;
            }
            return true;
        }
    }
}