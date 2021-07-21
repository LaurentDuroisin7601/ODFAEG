#include "../../../include/odfaeg/Graphics/baseChangementMatrix.h"
namespace odfaeg {
    namespace graphic {
        BaseChangementMatrix::BaseChangementMatrix() {
            matrix4f.identity();
            iso2DMatrix = false;
        }
        void BaseChangementMatrix::set2DIsoMatrix() {
            matrix4f.m11 = 0.5f;
            matrix4f.m12 = -1.f;
            matrix4f.m21 = 0.25f;
            matrix4f.m22 = 0.5f;
            invMatrix4f = matrix4f.inverse();
            iso2DMatrix = true;
        }
        math::Vec3f BaseChangementMatrix::changeOfBase(math::Vec3f v) {
            math::Vec3f p = matrix4f * v;
            p = math::Vec3f ((int) p.x, (int) p.y, (int) p.z);
            return p;
        }
        math::Vec3f BaseChangementMatrix::unchangeOfBase(math::Vec3f v) {
            math::Vec3f p = invMatrix4f * v;
            p = math::Vec3f ((int) p.x, (int) p.y, (int) p.z);
            return p;
        }
        math::Matrix4f BaseChangementMatrix::getMatrix() {
            return matrix4f;
        }
        bool BaseChangementMatrix::isIso2DMatrix() {
            return iso2DMatrix;
        }
    }
}
