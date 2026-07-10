module;
#include <vector>
#include <iostream>
//import odfaeg.graphic.camera;
module odfaeg.graphic.camera;
import odfaeg.graphic.projMatrix;
import odfaeg.graphic.viewMatrix;
import odfaeg.entity.transformMatrix;
import odfaeg.math.vec;
import odfaeg.math.maths;
import odfaeg.physic.boundingBox;
import odfaeg.math.computer;
namespace odfaeg {
    namespace graphic {

        Camera::Camera() : viewport(0, 0, 0, 2, 2, 1) {
            setPerspective(-1, 1, -1, 1, 0, 1);
            this->up = math::Vec3f(0.f, 1.f, 0.f);
            this->forward = math::Vec3f(0.f, 0.f, -1.f);
            center = math::Vec3f(0.f, 0.f, 0.f);
            this->left = forward.cross(up).normalize();   // X vers la gauche
            this->up = left.cross(forward).normalize();   // recalcule up pour ortho
            this->target = center + forward;
            width = height = 2;
            depth = 1;

            viewMatrix.setAxis(left, up, forward);
            viewMatrix.setCenter(center);
            teta = math::Math::toDegrees(math::Math::atang2(forward.x(), forward.z()));         // rotation horizontale autour de Y
            phi = math::Math::toDegrees(math::Math::asinus(forward.y()));
            gamma = 0;
            lockTeta = lockPhi = 0;
            viewUpdated = true;            
        }
        Camera::Camera(double width, double height, double zNear, double zFar) : viewport(0, 0, zNear, width, height, zFar), width(width), height(height), depth(zFar) {
            //std::cout<<"set perspective"<<std::endl;
            setPerspective(-width * 0.5f, width * 0.5f, -height * 0.5f, height * 0.5f, zNear, zFar);
            //std::cout<<"perspective set"<<std::endl;
            this->up = math::Vec3f(0.f, 1.f, 0.f);
            this->forward = math::Vec3f(0.f, 0.f, -1.f);
            center = math::Vec3f(0.f, 0.f, 0.f);
            this->left = forward.cross(up).normalize();   // X vers la gauche
            this->up = left.cross(forward).normalize();   // recalcule up pour ortho
            this->target = center + forward;            

            viewMatrix.setAxis(left, up, forward);
            viewMatrix.setCenter(center);
            teta = math::Math::toDegrees(math::Math::atang2(forward.x(), forward.z()));         // rotation horizontale autour de Y
            phi = math::Math::toDegrees(math::Math::asinus(forward.y()));
            gamma = 0;
            lockTeta = lockPhi = 0;
            viewUpdated = true;
            //std::cout<<"camera created"<<std::endl;            
        }   
        math::Vec3f Camera::getCenter() {
            return center;
        }
        void Camera::setUp(math::Vec3f up) {
            left = forward.cross(up).normalize();
            up = left.cross(forward).normalize();
            viewMatrix.setAxis(left, up, forward);
            this->up = up;
        }
        Camera::Camera(double width, double height, double fovy, double zNear, double zFar) : viewport(0, 0, zNear, width, height, zFar), width(width), height(height), depth(zFar) {
            setPerspective(fovy, width / height, zNear, zFar);
            this->center = math::Vec3f(0.f, 0.f, 0.f);
            this->forward = math::Vec3f(0.f, 0.f, -1.f);  // vers -Z
            this->up = math::Vec3f(0.f, 1.f, 0.f);        // Y en haut
            this->left = forward.cross(up).normalize();   // X vers la gauche
            this->up = left.cross(forward).normalize();   // recalcule up pour ortho
            this->target = center + forward;           
            viewMatrix.setAxis(left, up, forward);
            viewMatrix.setCenter(center);
            teta = math::Math::toDegrees(math::Math::atang2(forward.x(), forward.z()));         // rotation horizontale autour de Y
            phi = math::Math::toDegrees(math::Math::asinus(forward.y()));
            gamma = 0;
            lockTeta = lockPhi = 0;
            viewUpdated = true;            
        }
        math::Vec3f Camera::getSize() {
            return math::Vec3f(width, height, depth);
        }   
        void Camera::move(float x, float y, float z) {
            math::Vec3f d(x, y, z);
            center += d;
            target = center + forward;
            viewMatrix.setCenter(center);
            viewUpdated = true;
        }        
        void Camera::move(math::Vec3f d, float delta) {
            center += d * delta;
            target = center + forward;
            viewMatrix.setCenter(center);
            viewUpdated = true;
        }        
        physic::BoundingBox Camera::getViewVolume() {
            //std::cout<<"get view volume"<<std::endl;
            physic::BoundingBox viewVolume(-width * 0.5f,
                -height * 0.5f,
                -depth,
                width,
                height,
                depth*2);
            std::vector<math::Vec3f> vertices = viewVolume.getVertices();
            for (unsigned int i = 0; i < vertices.size(); i++) {                
                vertices[i] = getViewMatrix().inverseTransform(vertices[i]);                
            }
            std::array<std::array<float, 2>, 3> extends = math::Computer::getExtends(vertices);
            viewVolume = physic::BoundingBox(extends[0][0], extends[1][0], extends[2][0], extends[0][1] - extends[0][0], extends[1][1] - extends[1][0], extends[2][1] - extends[2][0]);            
            //std::cout<<"view volume : "<<viewVolume.getPosition()<<','<<viewVolume.getSize()<<std::endl;
            return viewVolume;
        }        
        ViewMatrix Camera::getViewMatrix() {            
            return viewMatrix;
        }
        ProjMatrix Camera::getProjMatrix() {            
            return projMatrix;
        }
        physic::BoundingBox Camera::getViewport() {
            return viewport;
        }
        void Camera::setViewport(physic::BoundingBox rect) {
            viewport = rect;
            viewUpdated = true;
        }
        math::Vec3f Camera::getLeft() {
            return left;
        }
        math::Vec3f Camera::getUp() {
            return up;
        }
        math::Vec3f Camera::getForward() {
            return forward;
        }
        float Camera::getTeta() {
            return teta;
        }
        float Camera::getPhi() {
            return phi;
        }
        void Camera::computeVectorsFromAngles() {
            forward = math::Math::toCartesian(math::Math::toRadians(teta), math::Math::toRadians(phi)).normalize();            
            left = forward.cross(up).normalize();
            target = center + forward;
            up = left.cross(forward).normalize();
        }
        void Camera::lookAt(float x, float y, float z, math::Vec3f up) {
            target = math::Vec3f(x, y, z);

            forward = target - center;
            forward = forward.normalize();
            left = forward.cross(up).normalize();
            up = left.cross(forward).normalize();
            viewMatrix.setAxis(left, up, forward);
            this->up = up;
            teta = math::Math::toDegrees(math::Math::atang2(forward.x(), forward.z()));         // rotation horizontale autour de Y
            phi = math::Math::toDegrees(math::Math::asinus(forward.y()));
            viewUpdated = true;
        }
        void Camera::rotate(float teta, float phi) {
            if (lockPhi != 0 && phi > lockPhi)
                phi = lockPhi;
            if (lockPhi != 0 && phi < -lockPhi)
                phi = -lockPhi;
            this->phi = phi;
            if (lockTeta != 0 && teta > lockTeta)
                teta = lockTeta;
            if (lockTeta != 0 && teta < -lockTeta)
                teta = -lockTeta;
            this->teta = teta;
            computeVectorsFromAngles();
            viewMatrix.setAxis(left, up, forward);
            viewUpdated = true;
        }
        void Camera::rotate(float gamma) {
            entity::TransformMatrix tm;
            tm.setRotation(forward, gamma);
            up = tm.transform(up);
            left = forward.cross(up).normalize();
            teta = math::Math::toDegrees(math::Math::atang2(forward.x(), forward.z()));         // rotation horizontale autour de Y
            phi = math::Math::toDegrees(math::Math::asinus(forward.y()));
            viewMatrix.setAxis(left, up, forward);
            viewUpdated = true;
        }
        void Camera::setPerspective(double left, double right, double bottom, double top, double front, double back) {
            projMatrix.setOrthoMatrix(left, right, bottom, top, front, back);
            viewUpdated = true;
            ortho = true;
        }
        void Camera::setPerspective(double fovy, double aspect, double front, double back) {            
            double tangent = math::Math::tang(math::Math::toRadians(fovy) * 0.5f);  
            double hheight = (front * tangent);          
            double hwidth = (hheight * aspect);
            projMatrix.setPerspectiveMatrix(-hwidth, hwidth, -hheight, hheight, front, back);
            viewUpdated = true;
            ortho = false;
        }
        bool Camera::isOrtho() {
            return ortho;
        }
        void Camera::setConstrains(float lockTeta, float lockPhi) {
            this->lockTeta = lockTeta;
            this->lockPhi = lockPhi;
        }
        void Camera::setCenter(math::Vec3f center) {
            this->center = center;
            viewMatrix.setCenter(center);
            viewUpdated = true;
        }
        float Camera::getGamma() {
            return gamma;
        }     
    }
}
