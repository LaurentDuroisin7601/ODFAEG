module;
//import odfaeg.graphic.viewMatrix;
module odfaeg.graphic.viewMatrix;
import odfaeg.math.vec;
namespace odfaeg {
    namespace graphic {
        ViewMatrix::ViewMatrix() {            
            needToUpdate = false;
            inverseNeedToUpdate = false;
        }
        void ViewMatrix::reset() {
            matrix4f.identity();
            invMat4f.identity();
            needToUpdate = false;
            inverseNeedToUpdate = false;
        }
        math::Vec4f ViewMatrix::transform(math::Vec4f vec4) {
            vec4[3] = 1;
            update();
            return matrix4f * vec4;
        }        
        void ViewMatrix::update() {
            if (needToUpdate) {                
                matrix4f[0][0] = xAxis.x();
                matrix4f[0][1] = xAxis.y();
                matrix4f[0][2] = xAxis.z();
                matrix4f[0][3] = -xAxis.dot(center);
                matrix4f[1][0] = yAxis.x();
                matrix4f[1][1] = yAxis.y();
                matrix4f[1][2] = yAxis.z();
                matrix4f[1][3] = -yAxis.dot(center);
                matrix4f[2][0] = zAxis.x();
                matrix4f[2][1] = zAxis.y();
                matrix4f[2][2] = zAxis.z();
                matrix4f[2][3] = -zAxis.dot(center);
                matrix4f[3][0] = 0;
                matrix4f[3][1] = 0;
                matrix4f[3][2] = 0;
                matrix4f[3][3] = 1;                
                needToUpdate = false;
            }
            if (inverseNeedToUpdate) {
                invMat4f = matrix4f.inverse();
                inverseNeedToUpdate = false;
            }
        }
        math::Matrix4f ViewMatrix::getMatrix() {
            update();
            return matrix4f;
        }
        math::Vec4f ViewMatrix::inverseTransform(math::Vec4f vec4) {
            vec4[3] = 1;
            update();
            return invMat4f * vec4;
        }        
        void ViewMatrix::setAxis(math::Vec3f left, math::Vec3f up, math::Vec3f forward) {
            xAxis = left.normalize();
            yAxis = up.normalize();
            zAxis = forward.normalize();            
            needToUpdate = true;
            inverseNeedToUpdate = true;
        }
        void ViewMatrix::setCenter(math::Vec3f center) {
            this->center = center;
            needToUpdate = true;
            inverseNeedToUpdate = true;
        }
    }
}