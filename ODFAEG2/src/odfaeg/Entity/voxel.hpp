#include "gameObject.hpp"
namespace odfaeg {
    namespace entity {
        class Voxel : public GameObject {
            public :
                Voxel(Cube& cube);  
        };
    }
}