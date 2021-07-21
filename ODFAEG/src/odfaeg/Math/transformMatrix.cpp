

#include "../../../include/odfaeg/Math/transformMatrix.h"
#include "../../../include/odfaeg/Math/computer.h"
namespace odfaeg {
    namespace graphic {
        TransformMatrix::TransformMatrix() {
            t3d = math::Vec3f(0, 0, 0);
            r3d = 0;
            r3dAxis = math::Vec3f(0, 0, 0);
            s3d = math::Vec3f (1, 1, 1);
            o3d = math::Vec3f (0, 0, 0);
            entityId=0;
            needToUpdate3D = false;
            inverseNeedToUpdate3D = false;
        }
        void TransformMatrix::setTransformId(unsigned int id) {
            this->id = id;
        }
        unsigned int TransformMatrix::getTransformId() {
            return id;
        }
        void TransformMatrix::setEntityId(unsigned int id) {
            entityId = id;
        }
        unsigned int TransformMatrix::getEntityId() {
            return entityId;
        }
        float* TransformMatrix::getGlMatrix () {
            float* matrix = new float[16];

            if (needToUpdate3D) {
                float angle;
                angle = math::Math::toRadians(r3d);
                matrix4f.m11 = (r3dAxis.x * r3dAxis.x * (1 - math::Math::cosinus(angle)) + math::Math::cosinus(angle)) * s3d.x;
                matrix4f.m12 = (r3dAxis.x * r3dAxis.y * (1 - math::Math::cosinus(angle)) - r3dAxis.z * math::Math::sinus(angle)) * s3d.y;
                matrix4f.m13 = (r3dAxis.x * r3dAxis.z * (1 - math::Math::cosinus(angle)) + r3dAxis.y * math::Math::sinus(angle)) * s3d.z;
                matrix4f.m14 = -o3d.x * matrix4f.m11 - o3d.y * matrix4f.m12 - o3d.z * matrix4f.m13 + t3d.x;
                matrix4f.m21 = (r3dAxis.y * r3dAxis.x * (1 - math::Math::cosinus(angle)) + r3dAxis.z * math::Math::sinus(angle)) * s3d.x;
                matrix4f.m22 = (r3dAxis.y * r3dAxis.y * (1 - math::Math::cosinus(angle)) + math::Math::cosinus(angle)) * s3d.y;
                matrix4f.m23 = (r3dAxis.y * r3dAxis.z * (1 - math::Math::cosinus(angle)) - r3dAxis.x * math::Math::sinus(angle)) * s3d.z;
                matrix4f.m24 = -o3d.x * matrix4f.m21 - o3d.y * matrix4f.m22 - o3d.z * matrix4f.m23 + t3d.y;
                matrix4f.m31 = (r3dAxis.z * r3dAxis.x * (1 - math::Math::cosinus(angle)) - r3dAxis.y * math::Math::sinus(angle)) * s3d.x;
                matrix4f.m32 = (r3dAxis.z * r3dAxis.y * (1 - math::Math::cosinus(angle)) + r3dAxis.x * math::Math::sinus(angle)) * s3d.y;
                matrix4f.m33 = (r3dAxis.z * r3dAxis.z * (1 - math::Math::cosinus(angle)) + math::Math::cosinus(angle)) * s3d.z;
                matrix4f.m34 = -o3d.x * matrix4f.m31 - o3d.y * matrix4f.m32 - o3d.z * matrix4f.m33 + t3d.z;
                matrix4f.m41 = 0;
                matrix4f.m42 = 0;
                matrix4f.m43 = 0;
                matrix4f.m44 = 1;
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
        void TransformMatrix::setOrigin(math::Vec3f vec3) {
            o3d = vec3;
            needToUpdate3D = true;
            inverseNeedToUpdate3D = true;
        }
        void TransformMatrix::setTranslation (const math::Vec3f trans) {
            t3d = trans;
            needToUpdate3D = true;
            inverseNeedToUpdate3D = true;
        }
        void TransformMatrix::setScale(const math::Vec3f scale) {
            s3d = scale;
            needToUpdate3D = true;
            inverseNeedToUpdate3D = true;
        }
        void TransformMatrix::setRotation(const math::Vec3f axis, float teta) {
            r3d = teta;
            r3dAxis = axis.normalize();
            needToUpdate3D = true;
            inverseNeedToUpdate3D = true;
        }
        void TransformMatrix::reset3D () {
            matrix4f.identity();
            needToUpdate3D = false;
        }
        float TransformMatrix::getRotation() {
            return r3d;
        }
        math::Vec3f TransformMatrix::getTranslation() {
            return t3d;
        }
        math::Vec3f TransformMatrix::getOrigin() {
            return o3d;
        }
        math::Vec3f TransformMatrix::getScale() {
            return s3d;
        }
        math::Vec3f TransformMatrix::transform (const math::Vec3f vec3) {
            if (needToUpdate3D) {
                float angle;
                angle = math::Math::toRadians(r3d);
                matrix4f.m11 = (r3dAxis.x * r3dAxis.x * (1 - math::Math::cosinus(angle)) + math::Math::cosinus(angle)) * s3d.x;
                matrix4f.m12 = (r3dAxis.x * r3dAxis.y * (1 - math::Math::cosinus(angle)) - r3dAxis.z * math::Math::sinus(angle)) * s3d.y;
                matrix4f.m13 = (r3dAxis.x * r3dAxis.z * (1 - math::Math::cosinus(angle)) + r3dAxis.y * math::Math::sinus(angle)) * s3d.z;
                matrix4f.m14 = -o3d.x * matrix4f.m11 - o3d.y * matrix4f.m12 - o3d.z * matrix4f.m13 + t3d.x;
                matrix4f.m21 = (r3dAxis.y * r3dAxis.x * (1 - math::Math::cosinus(angle)) + r3dAxis.z * math::Math::sinus(angle)) * s3d.x;
                matrix4f.m22 = (r3dAxis.y * r3dAxis.y * (1 - math::Math::cosinus(angle)) + math::Math::cosinus(angle)) * s3d.y;
                matrix4f.m23 = (r3dAxis.y * r3dAxis.z * (1 - math::Math::cosinus(angle)) - r3dAxis.x * math::Math::sinus(angle)) * s3d.z;
                matrix4f.m24 = -o3d.x * matrix4f.m21 - o3d.y * matrix4f.m22 - o3d.z * matrix4f.m23 + t3d.y;
                matrix4f.m31 = (r3dAxis.z * r3dAxis.x * (1 - math::Math::cosinus(angle)) - r3dAxis.y * math::Math::sinus(angle)) * s3d.x;
                matrix4f.m32 = (r3dAxis.z * r3dAxis.y * (1 - math::Math::cosinus(angle)) + r3dAxis.x * math::Math::sinus(angle)) * s3d.y;
                matrix4f.m33 = (r3dAxis.z * r3dAxis.z * (1 - math::Math::cosinus(angle)) + math::Math::cosinus(angle)) * s3d.z;
                matrix4f.m34 = -o3d.x * matrix4f.m31 - o3d.y * matrix4f.m32 - o3d.z * matrix4f.m33 + t3d.z;
                matrix4f.m41 = 0;
                matrix4f.m42 = 0;
                matrix4f.m43 = 0;
                matrix4f.m44 = 1;
                needToUpdate3D = false;
            }
            return matrix4f * vec3;
        }
        math::Matrix4f TransformMatrix::getMatrix () {
            update();
            return matrix4f;
        }
        math::Vec3f TransformMatrix::inverseTransform (const math::Vec3f vec3) {
            if (inverseNeedToUpdate3D) {
                try {
                    invMat4f = matrix4f.inverse();
                } catch (std::exception &e) {
                    invMat4f.identity();
                }
                inverseNeedToUpdate3D = false;
            }
            return invMat4f * vec3;
        }
        void TransformMatrix::combine(math::Matrix4f other) {
            needToUpdate3D = true;
            update();
            matrix4f =  matrix4f * other;
        }
        bool TransformMatrix::operator== (const TransformMatrix& other) {
            return matrix4f == other.matrix4f;
        }
        void TransformMatrix::setMatrix(math::Matrix4f matrix4f) {
            this->matrix4f = matrix4f;
        }
        void TransformMatrix::update() {
            if (needToUpdate3D) {
                float angle;
                angle = math::Math::toRadians(r3d);
                matrix4f.m11 = (r3dAxis.x * r3dAxis.x * (1 - math::Math::cosinus(angle)) + math::Math::cosinus(angle)) * s3d.x;
                matrix4f.m12 = (r3dAxis.x * r3dAxis.y * (1 - math::Math::cosinus(angle)) - r3dAxis.z * math::Math::sinus(angle)) * s3d.y;
                matrix4f.m13 = (r3dAxis.x * r3dAxis.z * (1 - math::Math::cosinus(angle)) + r3dAxis.y * math::Math::sinus(angle)) * s3d.z;
                matrix4f.m14 = -o3d.x * matrix4f.m11 - o3d.y * matrix4f.m12 - o3d.z * matrix4f.m13 + t3d.x;
                matrix4f.m21 = (r3dAxis.y * r3dAxis.x * (1 - math::Math::cosinus(angle)) + r3dAxis.z * math::Math::sinus(angle)) * s3d.x;
                matrix4f.m22 = (r3dAxis.y * r3dAxis.y * (1 - math::Math::cosinus(angle)) + math::Math::cosinus(angle)) * s3d.y;
                matrix4f.m23 = (r3dAxis.y * r3dAxis.z * (1 - math::Math::cosinus(angle)) - r3dAxis.x * math::Math::sinus(angle)) * s3d.z;
                matrix4f.m24 = -o3d.x * matrix4f.m21 - o3d.y * matrix4f.m22 - o3d.z * matrix4f.m23 + t3d.y;
                matrix4f.m31 = (r3dAxis.z * r3dAxis.x * (1 - math::Math::cosinus(angle)) - r3dAxis.y * math::Math::sinus(angle)) * s3d.x;
                matrix4f.m32 = (r3dAxis.z * r3dAxis.y * (1 - math::Math::cosinus(angle)) + r3dAxis.x * math::Math::sinus(angle)) * s3d.y;
                matrix4f.m33 = (r3dAxis.z * r3dAxis.z * (1 - math::Math::cosinus(angle)) + math::Math::cosinus(angle)) * s3d.z;
                matrix4f.m34 = -o3d.x * matrix4f.m31 - o3d.y * matrix4f.m32 - o3d.z * matrix4f.m33 + t3d.z;
                matrix4f.m41 = 0;
                matrix4f.m42 = 0;
                matrix4f.m43 = 0;
                matrix4f.m44 = 1;
                needToUpdate3D = false;
            }
        }
    }
}

