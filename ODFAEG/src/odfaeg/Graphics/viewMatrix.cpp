

#include "../../../include/odfaeg/Graphics/viewMatrix.h"
#include "../../../include/odfaeg/Physics/boundingBox.h"
#include "../../../include/odfaeg/Math/computer.h"
namespace odfaeg {
    namespace graphic {
        ViewMatrix::ViewMatrix() {
            s3d = math::Vec3f (1, 1, 1);
            o3d = math::Vec3f (0, 0, 0);
            angle = 0;
            needToUpdate3D = false;
            inverseNeedToUpdate3D = false;
        }
        void ViewMatrix::setScale(const math::Vec3f scale) {
            s3d = scale;
            needToUpdate3D = true;
            inverseNeedToUpdate3D = true;
        }
        void ViewMatrix::setOrigin(math::Vec3f origin) {
            o3d = origin;
            needToUpdate3D = true;
            inverseNeedToUpdate3D = true;
        }
        void ViewMatrix::reset3D () {
            matrix4f.identity();
            invMat4f.identity();
            needToUpdate3D = false;
        }
        math::Vec3f ViewMatrix::transform (const math::Vec3f vec3) {
            if (needToUpdate3D) {
                float a = math::Math::toRadians(angle);
                math::Matrix4f transform;
                transform.m11 = (zAxis.x * zAxis.x * (1 - math::Math::cosinus(a)) + math::Math::cosinus(a));
                transform.m12 = (zAxis.x * zAxis.y * (1 - math::Math::cosinus(a)) - zAxis.z * math::Math::sinus(a));
                transform.m13 = (zAxis.x * zAxis.z * (1 - math::Math::cosinus(a)) + zAxis.y * math::Math::sinus(a));
                transform.m14 = 0;
                transform.m21 = (zAxis.y * zAxis.x * (1 - math::Math::cosinus(a)) + zAxis.z * math::Math::sinus(a));
                transform.m22 = (zAxis.y * zAxis.y * (1 - math::Math::cosinus(a)) + math::Math::cosinus(a));
                transform.m23 = (zAxis.y * zAxis.z * (1 - math::Math::cosinus(a)) - zAxis.x * math::Math::sinus(a));
                transform.m24 = 0;
                transform.m31 = (zAxis.z * zAxis.x * (1 - math::Math::cosinus(a)) - zAxis.y * math::Math::sinus(a));
                transform.m32 = (zAxis.z * zAxis.y * (1 - math::Math::cosinus(a)) + zAxis.x * math::Math::cosinus(a));
                transform.m33 = (zAxis.z * zAxis.z * (1 - math::Math::cosinus(a)) + math::Math::cosinus(a));
                transform.m34 = 0;
                transform.m41 = 0;
                transform.m42 = 0;
                transform.m43 = 0;
                transform.m44 = 1;
                matrix4f.m11 = xAxis.x;
                matrix4f.m12 = xAxis.y;
                matrix4f.m13 = xAxis.z;
                matrix4f.m14 = -xAxis.dot2(o3d) / s3d.x;
                matrix4f.m21 = yAxis.x;
                matrix4f.m22 = yAxis.y;
                matrix4f.m23 = yAxis.z;
                matrix4f.m24 = -yAxis.dot2(o3d) / s3d.y;
                matrix4f.m31 = zAxis.x;
                matrix4f.m32 = zAxis.y;
                matrix4f.m33 = zAxis.z;
                matrix4f.m34 = -zAxis.dot2(o3d) / s3d.z;
                matrix4f.m41 = 0;
                matrix4f.m42 = 0;
                matrix4f.m43 = 0;
                matrix4f.m44 = 1;
                matrix4f = transform * matrix4f;
                needToUpdate3D = false;
            }
            return matrix4f * vec3;
        }
        float* ViewMatrix::getGlMatrix () {
            float* matrix = new float[16];
            if (needToUpdate3D) {
                float a = math::Math::toRadians(angle);
                math::Matrix4f transform;
                transform.m11 = (zAxis.x * zAxis.x * (1 - math::Math::cosinus(a)) + math::Math::cosinus(a));
                transform.m12 = (zAxis.x * zAxis.y * (1 - math::Math::cosinus(a)) - zAxis.z * math::Math::sinus(a));
                transform.m13 = (zAxis.x * zAxis.z * (1 - math::Math::cosinus(a)) + zAxis.y * math::Math::sinus(a));
                transform.m14 = 0;
                transform.m21 = (zAxis.y * zAxis.x * (1 - math::Math::cosinus(a)) + zAxis.z * math::Math::sinus(a));
                transform.m22 = (zAxis.y * zAxis.y * (1 - math::Math::cosinus(a)) + math::Math::cosinus(a));
                transform.m23 = (zAxis.y * zAxis.z * (1 - math::Math::cosinus(a)) - zAxis.x * math::Math::sinus(a));
                transform.m24 = 0;
                transform.m31 = (zAxis.z * zAxis.x * (1 - math::Math::cosinus(a)) - zAxis.y * math::Math::sinus(a));
                transform.m32 = (zAxis.z * zAxis.y * (1 - math::Math::cosinus(a)) + zAxis.x * math::Math::cosinus(a));
                transform.m33 = (zAxis.z * zAxis.z * (1 - math::Math::cosinus(a)) + math::Math::cosinus(a));
                transform.m34 = 0;
                transform.m41 = 0;
                transform.m42 = 0;
                transform.m43 = 0;
                transform.m44 = 1;
                matrix4f.m11 = xAxis.x;
                matrix4f.m12 = xAxis.y;
                matrix4f.m13 = xAxis.z;
                matrix4f.m14 = -xAxis.dot2(o3d) / s3d.x;
                matrix4f.m21 = yAxis.x;
                matrix4f.m22 = yAxis.y;
                matrix4f.m23 = yAxis.z;
                matrix4f.m24 = -yAxis.dot2(o3d) / s3d.y;
                matrix4f.m31 = zAxis.x;
                matrix4f.m32 = zAxis.y;
                matrix4f.m33 = zAxis.z;
                matrix4f.m34 = -zAxis.dot2(o3d) / s3d.z;
                matrix4f.m41 = 0;
                matrix4f.m42 = 0;
                matrix4f.m43 = 0;
                matrix4f.m44 = 1;
                matrix4f = transform * matrix4f;
                needToUpdate3D = false;
            }
            matrix[0] = matrix4f.m11;
            matrix[1] = matrix4f.m21;
            matrix[2] = matrix4f.m31;
            matrix[3] = matrix4f.m41;
            matrix[4] = matrix4f.m12;
            matrix[5] = matrix4f.m22;
            matrix[6] = matrix4f.m32;
            matrix[7] = matrix4f.m42;
            matrix[8] = matrix4f.m13;
            matrix[9] = matrix4f.m23;
            matrix[10] = matrix4f.m33;
            matrix[11] = matrix4f.m43;
            matrix[12] = matrix4f.m14;
            matrix[13] = matrix4f.m24;
            matrix[14] = matrix4f.m34;
            matrix[15] = matrix4f.m44;
            return matrix;
        }
        void ViewMatrix::update() {
            if (needToUpdate3D) {
                float a = math::Math::toRadians(angle);
                math::Matrix4f transform;
                transform.m11 = (zAxis.x * zAxis.x * (1 - math::Math::cosinus(a)) + math::Math::cosinus(a));
                transform.m12 = (zAxis.x * zAxis.y * (1 - math::Math::cosinus(a)) - zAxis.z * math::Math::sinus(a));
                transform.m13 = (zAxis.x * zAxis.z * (1 - math::Math::cosinus(a)) + zAxis.y * math::Math::sinus(a));
                transform.m14 = 0;
                transform.m21 = (zAxis.y * zAxis.x * (1 - math::Math::cosinus(a)) + zAxis.z * math::Math::sinus(a));
                transform.m22 = (zAxis.y * zAxis.y * (1 - math::Math::cosinus(a)) + math::Math::cosinus(a));
                transform.m23 = (zAxis.y * zAxis.z * (1 - math::Math::cosinus(a)) - zAxis.x * math::Math::sinus(a));
                transform.m24 = 0;
                transform.m31 = (zAxis.z * zAxis.x * (1 - math::Math::cosinus(a)) - zAxis.y * math::Math::sinus(a));
                transform.m32 = (zAxis.z * zAxis.y * (1 - math::Math::cosinus(a)) + zAxis.x * math::Math::cosinus(a));
                transform.m33 = (zAxis.z * zAxis.z * (1 - math::Math::cosinus(a)) + math::Math::cosinus(a));
                transform.m34 = 0;
                transform.m41 = 0;
                transform.m42 = 0;
                transform.m43 = 0;
                transform.m44 = 1;
                matrix4f.m11 = xAxis.x;
                matrix4f.m12 = xAxis.y;
                matrix4f.m13 = xAxis.z;
                matrix4f.m14 = -xAxis.dot2(o3d) / s3d.x;
                matrix4f.m21 = yAxis.x;
                matrix4f.m22 = yAxis.y;
                matrix4f.m23 = yAxis.z;
                matrix4f.m24 = -yAxis.dot2(o3d) / s3d.y;
                matrix4f.m31 = zAxis.x;
                matrix4f.m32 = zAxis.y;
                matrix4f.m33 = zAxis.z;
                matrix4f.m34 = -zAxis.dot2(o3d) / s3d.z;
                matrix4f.m41 = 0;
                matrix4f.m42 = 0;
                matrix4f.m43 = 0;
                matrix4f.m44 = 1;
                matrix4f = transform * matrix4f;
                needToUpdate3D = false;
            }
        }
        math::Matrix4f ViewMatrix::getMatrix () {
            update();
            return matrix4f;
        }
        math::Vec3f ViewMatrix::inverseTransform (const math::Vec3f vec3) {

            if (inverseNeedToUpdate3D) {
                if (needToUpdate3D) {
                    float a = math::Math::toRadians(angle);
                    math::Matrix4f transform;
                    transform.m11 = (zAxis.x * zAxis.x * (1 - math::Math::cosinus(a)) + math::Math::cosinus(a));
                    transform.m12 = (zAxis.x * zAxis.y * (1 - math::Math::cosinus(a)) - zAxis.z * math::Math::sinus(a));
                    transform.m13 = (zAxis.x * zAxis.z * (1 - math::Math::cosinus(a)) + zAxis.y * math::Math::sinus(a));
                    transform.m14 = 0;
                    transform.m21 = (zAxis.y * zAxis.x * (1 - math::Math::cosinus(a)) + zAxis.z * math::Math::sinus(a));
                    transform.m22 = (zAxis.y * zAxis.y * (1 - math::Math::cosinus(a)) + math::Math::cosinus(a));
                    transform.m23 = (zAxis.y * zAxis.z * (1 - math::Math::cosinus(a)) - zAxis.x * math::Math::sinus(a));
                    transform.m24 = 0;
                    transform.m31 = (zAxis.z * zAxis.x * (1 - math::Math::cosinus(a)) - zAxis.y * math::Math::sinus(a));
                    transform.m32 = (zAxis.z * zAxis.y * (1 - math::Math::cosinus(a)) + zAxis.x * math::Math::cosinus(a));
                    transform.m33 = (zAxis.z * zAxis.z * (1 - math::Math::cosinus(a)) + math::Math::cosinus(a));
                    transform.m34 = 0;
                    transform.m41 = 0;
                    transform.m42 = 0;
                    transform.m43 = 0;
                    transform.m44 = 1;
                    matrix4f.m11 = xAxis.x;
                    matrix4f.m12 = xAxis.y;
                    matrix4f.m13 = xAxis.z;
                    matrix4f.m14 = -xAxis.dot2(o3d) / s3d.x;
                    matrix4f.m21 = yAxis.x;
                    matrix4f.m22 = yAxis.y;
                    matrix4f.m23 = yAxis.z;
                    matrix4f.m24 = -yAxis.dot2(o3d) / s3d.y;
                    matrix4f.m31 = zAxis.x;
                    matrix4f.m32 = zAxis.y;
                    matrix4f.m33 = zAxis.z;
                    matrix4f.m34 = -zAxis.dot2(o3d) / s3d.z;
                    matrix4f.m41 = 0;
                    matrix4f.m42 = 0;
                    matrix4f.m43 = 0;
                    matrix4f.m44 = 1;
                    matrix4f = transform * matrix4f;
                    needToUpdate3D = false;
                }
                invMat4f = matrix4f.inverse();
                inverseNeedToUpdate3D = false;
            }
            return invMat4f * vec3;
        }
        void ViewMatrix::setRotation(float angle) {
            this->angle = angle;
        }
        void ViewMatrix::setAxis(math::Vec3f left, math::Vec3f up, math::Vec3f forward) {
            xAxis = left;
            yAxis = left.cross(forward);
            zAxis = forward;
        }
        void ViewMatrix::combine(math::Matrix4f other) {
            update();
            matrix4f =  matrix4f * other;
        }
    }
}

