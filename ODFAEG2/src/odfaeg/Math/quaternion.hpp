#ifndef ODFAEG_QUATERNION_HPP
#define ODFAEG_QUATERNION_HPP
#include "matrix.hpp"
#include "maths.hpp"
namespace odfaeg {
    namespace math {
        class Quaternion {
        public:
            float x, y, z, w;
            Quaternion();
            Quaternion(float x, float y, float z, float w);
            void fromAngles(float xAngle, float yAngle, float zAngle);           
            Quaternion normalize();
            Quaternion slerp(Quaternion q2, float changeAmount);
            Matrix4f toRotationMatrix();
        private:
            float norm();
        };
    }
}
#include "quaternion.inl"
#endif