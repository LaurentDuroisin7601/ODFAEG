#include "../../../include/odfaeg/Math/quaternion.hpp"
namespace odfaeg {
    namespace math {
        Quaternion::Quaternion() {
            x = y = z = 0;
            w = 1;
        }
        Quaternion::Quaternion (float x, float y, float z, float w) {
            this->x = x;
            this->y = y;
            this->z = z;
            this->w = w;
        }
        void Quaternion::fromAngles(float xAngle, float yAngle, float zAngle) {
            float angle;
            float sinY, sinZ, sinX, cosY, cosZ, cosX;
            angle = zAngle * 0.5f;
            sinZ = Math::sinus(angle);
            cosZ = Math::cosinus(angle);
            angle = yAngle * 0.5f;
            sinY = Math::sinus(angle);
            cosY = Math::cosinus(angle);
            angle = xAngle * 0.5f;
            sinX = Math::sinus(angle);
            cosX = Math::cosinus(angle);

            // variables used to reduce multiplication calls.
            float cosYXcosZ = cosY * cosZ;
            float sinYXsinZ = sinY * sinZ;
            float cosYXsinZ = cosY * sinZ;
            float sinYXcosZ = sinY * cosZ;

            w = (cosYXcosZ * cosX - sinYXsinZ * sinX);
            x = (cosYXcosZ * sinX + sinYXsinZ * cosX);
            y = (sinYXcosZ * cosX + cosYXsinZ * sinX);
            z = (cosYXsinZ * cosX - sinYXcosZ * sinX);

        }
        float Quaternion::norm() {
            return w * w + x * x + y * y + z * z;
        }
        Quaternion Quaternion::normalize() {
            Quaternion quaternion;
            float n =  Math::inversSqrt(norm());
            quaternion.x *= n;
            quaternion.y *= n;
            quaternion.z *= n;
            quaternion.w *= n;
            return quaternion;
        }
        Quaternion Quaternion::slerp(Quaternion q2, float changeAmount) {
            Quaternion quaternion;
            if (this->x == q2.x && this->y == q2.y && this->z == q2.z
                && this->w == q2.w) {
                quaternion.x = x;
                quaternion.y = y;
                quaternion.z = z;
                quaternion.w = w;
                return quaternion;
            }

            float result = (this->x * q2.x) + (this->y * q2.y) + (this->z * q2.z)
                    + (this->w * q2.w);

            if (result < 0.0f) {
                // Negate the second quaternion and the result of the dot product
                q2.x = -q2.x;
                q2.y = -q2.y;
                q2.z = -q2.z;
                q2.w = -q2.w;
                result = -result;
            }

            // Set the first and second scale for the interpolation
            float scale0 = 1 - changeAmount;
            float scale1 = changeAmount;

            // Check if the angle between the 2 quaternions was big enough to
            // warrant such calculations
            if ((1 - result) > 0.1f) {
                // Get the angle between the 2 quaternions, and then store the sin()
                // of that angle
                float theta = Math::acosinus(result);
                float invSinTheta = 1.f / Math::sinus(theta);

                // Calculate the scale for q1 and q2, according to the angle and
                // its sine
                scale0 = Math::sinus((1 - changeAmount) * theta) * invSinTheta;
                scale1 = Math::sinus((changeAmount * theta)) * invSinTheta;
            }

            // Calculate the x, y, z and w values for the quaternion by using a
            // special
            // form of linear interpolation for quaternions.
            quaternion.x = (scale0 * this->x) + (scale1 * q2.x);
            quaternion.y = (scale0 * this->y) + (scale1 * q2.y);
            quaternion.z = (scale0 * this->z) + (scale1 * q2.z);
            quaternion.w = (scale0 * this->w) + (scale1 * q2.w);
            return quaternion;
        }
        Matrix4f Quaternion::toRotationMatrix() {
            math::Matrix4f result;
            float n = norm();
            // we explicitly test norm against one here, saving a division
            // at the cost of a test and branch.  Is it worth it?
            float s = (n == 1.f) ? 2.f : (n > 0.f) ? 2.f / n : 0;

            // compute xs/ys/zs first to save 6 multiplications, since xs/ys/zs
            // will be used 2-4 times each.
            float xs = x * s;
            float ys = y * s;
            float zs = z * s;
            float xx = x * xs;
            float xy = x * ys;
            float xz = x * zs;
            float xw = w * xs;
            float yy = y * ys;
            float yz = y * zs;
            float yw = w * ys;
            float zz = z * zs;
            float zw = w * zs;

            // using s=2/norm (instead of 1/norm) saves 9 multiplications by 2 here
            result.m11 = 1 - (yy + zz);
            result.m12 = (xy - zw);
            result.m13 = (xz + yw);
            result.m21 = (xy + zw);
            result.m22 = 1 - (xx + zz);
            result.m23 = (yz - xw);
            result.m31 = (xz - yw);
            result.m32 = (yz + xw);
            result.m33 = 1 - (xx + yy);

            return result;
        }
    }
}
