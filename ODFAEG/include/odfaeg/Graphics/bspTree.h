#ifndef BSP_TREE
#define BSP_TREE
#include "cellMap.h"
#include "../Math/computer.h"
#include "../Math/plane.h"
#include "../Physics/boundingPolyhedron.h"
namespace odfaeg {
template <typename E>
class BspTree {
public :
    BspTree(std::vector<E*> entities);
    void initTree(std::vector<E*> entities);
    void addEntity(E* entity);
    void getEntitiesInVol(std::vector<E*>& entities, BoundingVolume& bv);
    void getEntities(std::vector<E*>& entities);
    bool isEntityContainer() {
        return true;
    }
private :
    std::unique_ptr<BspTree> rightChild;
    std::unique_ptr<BspTree> leftChild;
    std::unique_ptr<CellMap<E>> cell;
    Plane separatingPlane;
    BaseChangementMatrix bm;
    bool leaf;
};
}
#include "bspTree.tpp"
#endif // BSP_TREE
