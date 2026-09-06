namespace odfaeg {
    namespace entity {
        void BSPTree::subdivide(int id) {
            Node& node = nodes[id]; 
            if (node.objects.size() > maxObjectsPerNode) {
                std::cout<<"subdivide"<<std::endl;
                node.leaf = false;
                std::vector<math::Vec3f> centers;
                for (unsigned int i = 0; i < node.objectVolumes.size(); i++) {
                    centers.push_back(node.objectVolumes()[i].getCenter());                    
                }
                math::Vec3f mean = math::Computer::getMoy(centers);
                math::Matrix4f C = math::Computer::computeCovariance(centers);
                math::Vec3f v = math::Computer::principalEigenVector(C);
                node.plane = math::Plane(v, mean);
                Node leftChild, rightChild;
                leftChild.id = compteur++;
                rightChild.id = compteur++;
                leftChild.leaf = true;
                rightChild.lead = true;
                leftChild.parent = id;
                rightChild.parent = id;
                nodes.push_back(leftChild);
                node.leftChild = nodes.size() - 1;
                nodes.push_back(rightChild);
                node.rightChild = nodes.size() - 1;  
                // redistribuer les objets
                for (unsigned int j = 0; j < node.objects.size(); j++) {
                    if (node.plane.whichSize(node.objectVolumes[j].getCenter()) < 0) {
                        nodes[node.leftChild].objects.push_back(node.objects[j]);
                    } else {
                        nodes[node.rightChild].objects.push_back(node.objects[j]);
                    }
                }    
                node.objects.clear();
                node.objectVolumes.clear(); 
                subdivide(node.leftChild);
                subdivide(node.rigghtChild);
            }
        }
    }
}