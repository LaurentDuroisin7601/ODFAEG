

#include "../../../include/odfaeg/Math/transformMatrix.h"
#include "../../../include/odfaeg/Math/computer.h"
namespace odfaeg {
    namespace graphic {
        TransformMatrix::TransformMatrix() {
            t3d = math::Vec3f(0.f, 0.f, 0.f);
            r3d = 0;
            r3dAxis = math::Vec3f(0.f, 0.f, 1.f);
            s3d = math::Vec3f (1.f, 1.f, 1.f);
            o3d = math::Vec3f (0.f, 0.f, 0.f);
            entityId=0;
            needToUpdate3D = true;
            inverseNeedToUpdate3D = true;
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
            update();
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
        math::Vec4f TransformMatrix::transform (const math::Vec4f vec4) {
            vec4[3] = 1;
            update();
            return matrix4f * vec4;
        }
        math::Matrix4f TransformMatrix::getMatrix () {
            update();
            return matrix4f;
        }
        math::Vec4f TransformMatrix::inverseTransform (const math::Vec4f vec4) {
            vec4[3] = 1;
            if (inverseNeedToUpdate3D) {
                try {
                    update();
                    invMat4f = matrix4f.inverse();
                } catch (std::exception &e) {
                    invMat4f.identity();
                }
                inverseNeedToUpdate3D = false;
            }
            return invMat4f * vec4;
        }
        void TransformMatrix::combine(math::Matrix4f other) {
            update();
            inverseNeedToUpdate3D = true;
            matrix4f =  matrix4f * other;
        }
        bool TransformMatrix::operator== (const TransformMatrix& other) const {
            return matrix4f == other.matrix4f;
        }
        void TransformMatrix::setMatrix(math::Matrix4f matrix4f) {
            this->matrix4f = matrix4f;
        }
        void TransformMatrix::update() {
            if (needToUpdate3D) {
                float angle;
                angle = math::Math::toRadians(r3d);
                matrix4f[0][0] = (r3dAxis.x() * r3dAxis.x() * (1 - math::Math::cosinus(angle)) + math::Math::cosinus(angle)) * s3d.x();
                matrix4f[0][1] = (r3dAxis.x() * r3dAxis.y() * (1 - math::Math::cosinus(angle)) - r3dAxis.z() * math::Math::sinus(angle)) * s3d.y();
                matrix4f[0][2] = (r3dAxis.x() * r3dAxis.z() * (1 - math::Math::cosinus(angle)) + r3dAxis.y() * math::Math::sinus(angle)) * s3d.z();
                matrix4f[0][3] = -o3d.x() * matrix4f[0][0] - o3d.y() * matrix4f[0][1] - o3d.z() * matrix4f[0][2] + t3d.x();
                matrix4f[1][0] = (r3dAxis.y() * r3dAxis.x() * (1 - math::Math::cosinus(angle)) + r3dAxis.z() * math::Math::sinus(angle)) * s3d.x();
                matrix4f[1][1] = (r3dAxis.y() * r3dAxis.y() * (1 - math::Math::cosinus(angle)) + math::Math::cosinus(angle)) * s3d.y();
                matrix4f[1][2] = (r3dAxis.y() * r3dAxis.z() * (1 - math::Math::cosinus(angle)) - r3dAxis.x() * math::Math::sinus(angle)) * s3d.z();
                matrix4f[1][3] = -o3d.x() * matrix4f[1][0] - o3d.y() * matrix4f[1][1] - o3d.z() * matrix4f[1][2] + t3d.y();
                matrix4f[2][0] = (r3dAxis.z() * r3dAxis.x() * (1 - math::Math::cosinus(angle)) - r3dAxis.y() * math::Math::sinus(angle)) * s3d.x();
                matrix4f[2][1] = (r3dAxis.z() * r3dAxis.y() * (1 - math::Math::cosinus(angle)) + r3dAxis.x() * math::Math::sinus(angle)) * s3d.y();
                matrix4f[2][2] = (r3dAxis.z() * r3dAxis.z() * (1 - math::Math::cosinus(angle)) + math::Math::cosinus(angle)) * s3d.z();
                matrix4f[2][3] = -o3d.x() * matrix4f[2][0] - o3d.y() * matrix4f[2][1] - o3d.z() * matrix4f[2][2] + t3d.z();
                matrix4f[3][0] = 0;
                matrix4f[3][1] = 0;
                matrix4f[3][2] = 0;
                matrix4f[3][3] = 1;
                //////std::cout<<"matrix : "<<matrix4f<<std::endl;
                needToUpdate3D = false;
            }
        }
    }
}

