#include "../../../include/odfaeg/Graphics/baseChangementMatrix.h"
namespace odfaeg {
    namespace graphic {
        BaseChangementMatrix::BaseChangementMatrix() {
            matrix4f.identity();
            invMatrix4f = matrix4f.inverse();
            iso2DMatrix = false;
        }
        void BaseChangementMatrix::set2DIsoMatrix() {
            matrix4f[0][0] = 0.5f;
            matrix4f[0][1] = -1.f;
            matrix4f[1][0] = 0.25f;
            matrix4f[1][1] = 0.5f;

            invMatrix4f = matrix4f.inverse();
            iso2DMatrix = true;
        }
        math::Vec4f BaseChangementMatrix::changeOfBase(math::Vec4f v) {
            v[3] = 1;
            return matrix4f * v;
        }
        math::Vec4f BaseChangementMatrix::unchangeOfBase(math::Vec4f v) {
            v[3] = 1;
            return invMatrix4f * v;
        }
        math::Matrix4f BaseChangementMatrix::getMatrix() {
            return matrix4f;
        }
        bool BaseChangementMatrix::isIso2DMatrix() {
            return iso2DMatrix;
        }
    }
}
