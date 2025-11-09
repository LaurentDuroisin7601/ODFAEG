

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
        math::Vec4f ViewMatrix::transform (const math::Vec4f vec4) {
            vec4[3] = 1;
            update();
            return matrix4f * vec4;
        }
        float* ViewMatrix::getGlMatrix () {
            float* matrix = new float[16];
            update();
            matrix[0] = matrix4f[0][0];
            matrix[1] = matrix4f[1][0];
            matrix[2] = matrix4f[2][0];
            matrix[3] = matrix4f[3][0];
            matrix[4] = matrix4f[0][1];
            matrix[5] = matrix4f[1][1];
            matrix[6] = matrix4f[2][1];
            matrix[7] = matrix4f[3][1];
            matrix[8] = matrix4f[0][2];
            matrix[9] = matrix4f[1][2];
            matrix[10] = matrix4f[2][2];
            matrix[11] = matrix4f[3][2];
            matrix[12] = matrix4f[0][3];
            matrix[13] = matrix4f[1][3];
            matrix[14] = matrix4f[2][3];
            matrix[15] = matrix4f[3][3];
            return matrix;
        }
        void ViewMatrix::update() {
            if (needToUpdate3D) {
                float a = math::Math::toRadians(angle);
                math::Matrix4f transform;
                transform[0][0] = (zAxis.x() * zAxis.x() * (1 - math::Math::cosinus(a)) + math::Math::cosinus(a));
                transform[0][1] = (zAxis.x() * zAxis.y() * (1 - math::Math::cosinus(a)) - zAxis.z() * math::Math::sinus(a));
                transform[0][2] = (zAxis.x() * zAxis.z() * (1 - math::Math::cosinus(a)) + zAxis.y() * math::Math::sinus(a));
                transform[0][3] = 0;
                transform[1][0] = (zAxis.y() * zAxis.x() * (1 - math::Math::cosinus(a)) + zAxis.z() * math::Math::sinus(a));
                transform[1][1] = (zAxis.y() * zAxis.y() * (1 - math::Math::cosinus(a)) + math::Math::cosinus(a));
                transform[1][2] = (zAxis.y() * zAxis.z() * (1 - math::Math::cosinus(a)) - zAxis.x() * math::Math::sinus(a));
                transform[1][3] = 0;
                transform[2][0] = (zAxis.z() * zAxis.x() * (1 - math::Math::cosinus(a)) - zAxis.y() * math::Math::sinus(a));
                transform[2][1] = (zAxis.z() * zAxis.y() * (1 - math::Math::cosinus(a)) + zAxis.x() * math::Math::sinus(a));
                transform[2][2] = (zAxis.z() * zAxis.z() * (1 - math::Math::cosinus(a)) + math::Math::cosinus(a));
                transform[2][3] = 0;
                transform[3][0] = 0;
                transform[3][1] = 0;
                transform[3][2] = 0;
                transform[3][3] = 1;
                matrix4f[0][0] = xAxis.x();
                matrix4f[0][1] = xAxis.y();
                matrix4f[0][2] = xAxis.z();
                matrix4f[0][3] = -xAxis.dot(o3d) / s3d.x();
                matrix4f[1][0] = yAxis.x();
                matrix4f[1][1] = yAxis.y();
                matrix4f[1][2] = yAxis.z();
                matrix4f[1][3] = -yAxis.dot(o3d) / s3d.y();
                matrix4f[2][0] = zAxis.x();
                matrix4f[2][1] = zAxis.y();
                matrix4f[2][2] = zAxis.z();
                matrix4f[2][3] = -zAxis.dot(o3d) / s3d.z();
                matrix4f[3][0] = 0;
                matrix4f[3][1] = 0;
                matrix4f[3][2] = 0;
                matrix4f[3][3] = 1;
                matrix4f = transform * matrix4f;
                needToUpdate3D = false;
            }
            if (inverseNeedToUpdate3D) {
                invMat4f = matrix4f.inverse();
                inverseNeedToUpdate3D = false;
            }
        }
        math::Matrix4f ViewMatrix::getMatrix () {
            update();
            return matrix4f;
        }
        math::Vec4f ViewMatrix::inverseTransform (const math::Vec4f vec4) {
            vec4[3] = 1;
            update();
            return invMat4f * vec4;
        }
        void ViewMatrix::setRotation(float angle) {
            this->angle = angle;
            needToUpdate3D = true;
            inverseNeedToUpdate3D = true;
        }
        void ViewMatrix::setAxis(math::Vec3f left, math::Vec3f up, math::Vec3f forward) {
            xAxis = left.normalize();
            yAxis = up.normalize();
            zAxis = forward.normalize();
            //////std::cout<<"axis : "<<xAxis<<" "<<yAxis<<" "<<zAxis<<std::endl;
            needToUpdate3D = true;
            inverseNeedToUpdate3D = true;
        }
        void ViewMatrix::combine(math::Matrix4f other) {
            update();
            matrix4f =  matrix4f * other;
        }
    }
}

