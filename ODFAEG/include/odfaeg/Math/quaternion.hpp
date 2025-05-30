#ifndef ODFAEG_QUATERNION_HPP
#define ODFAEG_QUATERNION_HPP
#include "../Math/matrix4.h"
namespace odfaeg {
    namespace math {
        class ODFAEG_MATH_API Quaternion {
        public :
            float x, y, z, w;
            Quaternion();
            Quaternion(float x, float y, float z, float w);
            Quaternion fromAngles(float xAngle, float yAngle, float zAngle);
            Matrix4f toRotationMatrix();
            Quaternion slerp(Quaternion other, float changeAmount);
            Quaternion normalize();
        private :
            float norm();
        };
    }
}
#endif // ODFAEG_QUATERNION_HPP
