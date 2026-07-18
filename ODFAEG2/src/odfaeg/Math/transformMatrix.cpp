module;
#include <exception>
#include <iostream>
//import odfaeg.entity.transformMatrix;
module odfaeg.math.transformMatrix;
import odfaeg.math.vec;
import odfaeg.math.maths;
import odfaeg.math.matrix;
namespace odfaeg {
    namespace math {
        TransformMatrix::TransformMatrix() {
            t = math::Vec3f(0.f, 0.f, 0.f);
            rAngle = 0;
            rAxis = math::Vec3f(0.f, 0.f, 1.f);
            s = math::Vec3f (1.f, 1.f, 1.f);
            o = math::Vec3f (0.f, 0.f, 0.f);            
            needToUpdate = true;
            inverseNeedToUpdate = true;
        } 
        void TransformMatrix::setOrigin(math::Vec3f vec3) {
            o = vec3;
            needToUpdate = true;
            inverseNeedToUpdate = true;
        }
        void TransformMatrix::setTranslation (const math::Vec3f trans) {
            t = trans;
            needToUpdate = true;
            inverseNeedToUpdate = true;
        }
        void TransformMatrix::setScale(const math::Vec3f scale) {
            s = scale;
            needToUpdate = true;
            inverseNeedToUpdate = true;
        }
        void TransformMatrix::setRotation(const math::Vec3f axis, float teta) {
            rAngle = teta;
            rAxis = axis.normalize();
            needToUpdate = true;
            inverseNeedToUpdate = true;
        }
        void TransformMatrix::reset () {
            matrix4f.identity();
            invMat4f.identity();
            needToUpdate = false;
            needToUpdate = false;
        }
        float TransformMatrix::getRotation() {
            return rAngle;
        }
        math::Vec3f TransformMatrix::getTranslation() {
            return t;
        }
        math::Vec3f TransformMatrix::getOrigin() {
            return o;
        }
        math::Vec3f TransformMatrix::getScale() {
            return s;
        }
        math::Vec4f TransformMatrix::transform (math::Vec4f vec4) {
            vec4[3] = 1;
            update();
            return matrix4f * vec4;
        }
        math::Matrix4f TransformMatrix::getMatrix () {
            update();
            return matrix4f;
        }
        math::Vec4f TransformMatrix::inverseTransform (math::Vec4f vec4) {
            vec4[3] = 1;
            update();
            return invMat4f * vec4;
        }
        void TransformMatrix::combine(math::Matrix4f other) {
            update();            
            matrix4f = matrix4f * other;
            invMat4f = matrix4f.inverse();
            needToUpdate = false;
            inverseNeedToUpdate = false;
        }
        bool TransformMatrix::operator== (const TransformMatrix& other) const {
            return matrix4f == other.matrix4f;
        }
        void TransformMatrix::setMatrix(math::Matrix4f matrix4f) {
            needToUpdate = false;
            this->matrix4f = matrix4f;
            invMat4f = matrix4f.inverse();
        }
        void TransformMatrix::update() {
            if (needToUpdate) {
                float angle;
                angle = math::Math::toRadians(rAngle);
                /*std::cout<<"rAxis  : "<<rAxis<<std::endl;
                std::cout<<"translation : "<<t<<std::endl;*/
                //std::cout<<"update"<<std::endl;
                matrix4f[0][0] = (rAxis.x() * rAxis.x() * (1 - math::Math::cosinus(angle)) + math::Math::cosinus(angle)) * s.x();
                matrix4f[0][1] = (rAxis.x() * rAxis.y() * (1 - math::Math::cosinus(angle)) - rAxis.z() * math::Math::sinus(angle)) * s.y();
                matrix4f[0][2] = (rAxis.x() * rAxis.z() * (1 - math::Math::cosinus(angle)) + rAxis.y() * math::Math::sinus(angle)) * s.z();
                matrix4f[0][3] = -o.x() * matrix4f[0][0] - o.y() * matrix4f[0][1] - o.z() * matrix4f[0][2] + t.x();
                matrix4f[1][0] = (rAxis.y() * rAxis.x() * (1 - math::Math::cosinus(angle)) + rAxis.z() * math::Math::sinus(angle)) * s.x();
                matrix4f[1][1] = (rAxis.y() * rAxis.y() * (1 - math::Math::cosinus(angle)) + math::Math::cosinus(angle)) * s.y();
                matrix4f[1][2] = (rAxis.y() * rAxis.z() * (1 - math::Math::cosinus(angle)) - rAxis.x() * math::Math::sinus(angle)) * s.z();
                matrix4f[1][3] = -o.x() * matrix4f[1][0] - o.y() * matrix4f[1][1] - o.z() * matrix4f[1][2] + t.y();
                matrix4f[2][0] = (rAxis.z() * rAxis.x() * (1 - math::Math::cosinus(angle)) - rAxis.y() * math::Math::sinus(angle)) * s.x();
                matrix4f[2][1] = (rAxis.z() * rAxis.y() * (1 - math::Math::cosinus(angle)) + rAxis.x() * math::Math::sinus(angle)) * s.y();
                matrix4f[2][2] = (rAxis.z() * rAxis.z() * (1 - math::Math::cosinus(angle)) + math::Math::cosinus(angle)) * s.z();
                //std::cout<<"scale : "<<s<<std::endl;
                matrix4f[2][3] = -o.x() * matrix4f[2][0] - o.y() * matrix4f[2][1] - o.z() * matrix4f[2][2] + t.z();
                matrix4f[3][0] = 0;
                matrix4f[3][1] = 0;
                matrix4f[3][2] = 0;
                matrix4f[3][3] = 1;
                //std::cout<<"matrix : "<<matrix4f<<std::endl;
                needToUpdate = false;
            }        
            if (inverseNeedToUpdate) {
                try {
                    invMat4f = matrix4f.inverse();
                }
                catch (std::exception& e) {
                    invMat4f.identity();
                }
                inverseNeedToUpdate = false;
            }
        }
    }
}