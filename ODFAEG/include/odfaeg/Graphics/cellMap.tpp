namespace odfaeg {
using namespace std;
template <typename E>
CellMap<E>::CellMap (BoundingPolyhedron* bp, Vec3f coords) {
    cellVolume = bp;
    center = bp->getCenter();
    passable = true;
    stateChanged = false;
    traveled = false;
    this->coords = coords;
}
template <typename E>
Vec3f CellMap<E>::getCoords () {
    return coords;
}
template <typename E>
void CellMap<E>::addEntity (E *entity) {

    entityInside.push_back(entity);
}
template <typename E>
BoundingPolyhedron* CellMap<E>::getCellVolume () {
    return cellVolume;
}
template <typename E>
bool CellMap<E>::isEntityInside () {
    if (entityInside.size() != 0)
            return true;
    return false;
}
template <typename E>
vector<E*> CellMap<E>::getEntitiesInside () {
    return entityInside;
}
template <typename E>
bool CellMap<E>::removeEntity (E *entity) {
    typename vector<E*>::iterator it;
    for (it = entityInside.begin(); it != entityInside.end();) {
        if (entity == *it) {
            it = entityInside.erase(it);
            return true;
        } else
            it++;

    }
    return false;
}
template <typename E>
Vec3f& CellMap<E>::getCenter () {
    return center;
}
template <typename E>
bool CellMap<E>::isTraveled () {
    return traveled;
}
template <typename E>
void CellMap<E>::setTraveled (bool traveled) {
    this->traveled = traveled;
}
template <typename E>
E* CellMap<E>::getEntityInside (int index) {
    if (index >= 0 && index < entityInside.size()) {
        return entityInside[index];
    }
    return nullptr;
}
template <typename E>
bool CellMap<E>::containsEntity (E *entity) {
    for (int i = 0; i < entityInside.size(); i++)
        if (*entityInside[i] == *entity)
            return true;
    return false;
}
template <typename E>
bool CellMap<E>::isPassable () {
    return passable;
}
template <typename E>
void CellMap<E>::setPassable (bool passable) {
    this->passable = passable;
}
template <typename E>
void CellMap<E>::setStateChanged (bool b) {
    this->stateChanged = b;
}
template <typename E>
bool CellMap<E>::isStateChanged () {
    return stateChanged;
}
template <typename E>
bool CellMap<E>::operator== (const CellMap<E> &cellMap) {
    return *cellVolume== *cellVolume;
}
template <typename E>
CellMap<E>::~CellMap () {
    entityInside.clear();

}
}


