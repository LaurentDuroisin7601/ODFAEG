#ifndef ODFAEG_COLLISION_RESULT_SET
#define ODFAEG_COLLISION_RESULT_SET
#include <vector>
#include "../Math/vec4.h"
namespace odfaeg {
    namespace graphic {
        class Entity;
    }
    namespace physic {
        class CollisionResultSet {
            public :
            struct Info {
                Info() {
                    entity = nullptr;
                    center = math::Vec3f(0, 0, 0);
                    mtu = math::Vec3f(0, 0, 0);
                    nearestVertexIndex1 = -1;
                    nearestPtIndex1 = -1;
                    nearestEdgeIndex1 = -1;
                    nearestFaceIndex1 = -1;
                    nearestVertexIndex2 = -1;
                    nearestPtIndex2 = -1;
                    nearestEdgeIndex2 = -1;
                    nearestFaceIndex2 = -1;
                }
                math::Vec3f mtu, center;
                graphic::Entity* entity;
                int nearestVertexIndex1;
                int nearestPtIndex1;
                int nearestEdgeIndex1;
                int nearestFaceIndex1;
                int nearestVertexIndex2;
                int nearestPtIndex2;
                int nearestEdgeIndex2;
                int nearestFaceIndex2;
            };
            static void pushCollisionInfo(Info info) {
                infos.push_back(info);
            }
            static Info popCollisionInfo() {
                Info info;
                if (infos.size() > 0) {
                    info = infos.back();
                    infos.pop_back();
                }
                return info;
            }
            static void clear() {
                infos.clear();
            }
            private :
            static std::vector<Info> infos;
        };
    }
}
#endif
