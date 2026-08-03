module;
export module odfaeg.math.quaternion;
import odfaeg.math.matrix;
import odfaeg.math.maths;
export namespace odfaeg {
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
        private :
            float norm();
        };
    }
}
module : private;
#include "quaternion.inl"