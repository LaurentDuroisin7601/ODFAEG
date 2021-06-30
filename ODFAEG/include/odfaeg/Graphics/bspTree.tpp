namespace odfaeg {
template <typename E>
BspTree<E>::BspTree(std::vector<E*> entities) {
    initTree(entities, gm);
}
template <typename E>
void BspTree<E>::initTree(std::vector<E*> entities) {
    bm.set2DIsoMatrix();
    std::vector<Vec3f> points;
    for (unsigned int i = 0; i < entities.size(); i++) {
       std::array<Vec3f, 8> vertices = entities[i]->getGlobalBounds().getVertices();
       for (unsigned int j = 0; j < vertices.size(); j++) {
            points.push_back(vertices[i]);
       }
    }
    std::array<std::array<float, 2>,3> tab = Computer::getExtends(points);
    std::array<Vec3f, 8> pts;
    pts[0] = Vec3f(tab[0][0], tab[1][0], tab[2][0]);
    pts[1] = Vec3f(tab[0][0] + tab[0][1], tab[1][0], tab[2][0]);
    pts[2] = Vec3f(tab[0][0] + tab[0][1], tab[1][0] + tab[1][1], tab[2][0]);
    pts[3] = Vec3f(tab[0][0], tab[1][0] + tab[1][1], tab[2][0]);
    pts[4] = Vec3f(tab[0][0], tab[1][0], tab[2][0]);
    pts[5] = Vec3f(tab[0][0] + tab[0][1], tab[1][0], tab[2][0] + tab[2][1]);
    pts[6] = Vec3f(tab[0][0] + tab[0][1], tab[1][0] + tab[1][1], tab[2][0] + tab[2][1]);
    pts[7] = Vec3f(tab[0][0], tab[1][0] + tab[1][1], tab[2][0] + tab[2][1]);
    BoundingPolyhedron* bp = new BoudningPolyhedron();
    for (unsigned int i = 0; i < pts.size(); i++) {
        pts[i] = bm.changeOfBase(pts[i]);
        bp->addPoint(pts[i]);
    }
    Vec3f coords = bp->getCenter();
    cell = std::make_unique<CellMap<E>>(bp, coords)
    for (unsigned int i = 0; i < entities.size(); i++) {
         cell->addEntity(entity);
    }
    if (entities.size() > 1) {
        std::vector<E*> leftEntities;
        std::vector<E*> rightEntities;
        std::vector<Vec3f> centers;
        for (unsigned int i = 0; i < entities.size(); i++) {
            centers.push_back(entities[i]->getCenter());
        }
        separatingPlane = Computer::computeSeparatingPlane(centers);
        for (unsigned int i = 0; i < entities.size(); i++) {
            float whichSide = separatingPlane.whichSide(entities[i]->getCenter());
            if (whichSide < 0) {
                leftEntities.push_back(entities[i]);
            } else if (whichSide > 0){
                rightEntities.push_back(entities[i]);
            } else {
                if (leftEntities.size() < leftEntities.size()) {
                    leftEntities.push_back(entities[i]);
                } else {
                    rightEntities.push_back(entities[i]);
                }
            }
        }
        leaf = false;
        BspTree<E>* leftChild = new BspTree<E>(leftEntities, gm);
        leftChild.reset(leftChild);
        BspTree<E>* rightChild = new BspTree<E>(rightEntities, gm);
        rightChild .reset(rightChild);
    } else {
        leaf = true;
    }
}
template <typename E>
void BspTree<E>::addEntity(E* entity) {
     std::vector<E*> entities;
     getEntities(entities);
     entities.push_back(entity)
     initTree(entities);
}
template <typename E>
void BspTree<E>::getEntitiesInVol(std::vector<E>& entities, BoundingVolume& bv)
    if(bv.intersects(*cell->getCellVolume()) && !isLeaf) {
        leftChild->getEntities(entities, bv);
        rightChild->getEntities(entities, bv);
    } else if (bv.intersects(*cell->getCellVolume()) && isLeaf) {
        std::vector<E*> entitiesInside = cell->getEntitiesInside();
        for (unsigned int i = 0; i < entitiesInside.size(); i++) {
            entities.push_back(entitiesInside[i]);
        }
    }
}
template <typename E>
void BspTree<E>::getEntities(std::vector<E>& entities) {
    if (!leaf) {
        leftChild->getEntities(entities);
        rightChild->getEntities(entities);
    } else {
        std::vector<E*> entitiesInside = cell->getEntitiesInside();
        for (unsigned int i = 0; i < entitiesInside.size(); i++) {
            entities.push_back(entitiesInside[i]);
        }
    }
}
}
