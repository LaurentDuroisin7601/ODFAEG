#include "../../../include/odfaeg/Graphics/view.h"

#include <SFML/OpenGL.hpp>
namespace odfaeg {
    namespace graphic {
        using namespace sf;
        View::View () : viewport(0, 0, 0, 2, 2, 1), depth(1) {
            setPerspective(-1, 1, -1, 1, 0, 1);
            up = math::Vec3f(0, 1, 0);
            position = math::Vec3f (0, 0, 0);
            teta = phi = gamma = 0;
            forward = math::Math::toCartesian(phi, teta).normalize();
            left = forward.cross(up).normalize();
            target = position + forward;
            viewMatrix.setScale(math::Vec3f(1.f, 1.f, 1.f));
            viewMatrix.setAxis(left, up, forward);
            viewMatrix.setOrigin(position);
            lockTeta = lockPhi = 0;
            viewUpdated = true;
            flipX = flipY = false;
        }
        View::View (double width, double height, double zNear, double zFar) : viewport(0, 0, zNear, width, height, zFar), depth(zFar) {

            setPerspective(-width * 0.5f, width * 0.5f, -height * 0.5f, height * 0.5f, zNear, zFar);
            this->up = math::Vec3f(0, 1, 0);
            position = math::Vec3f (0, 0, 0);
            math::Vec3f upToXYPlane(up.x, up.y, 0);
            teta = upToXYPlane.getAngleBetween(math::Vec3f::yAxis, math::Vec3f::yAxis);
            math::Vec3f upToYZPlane(0, up.y, math::Math::abs(up.z));
            phi = upToYZPlane.getAngleBetween(math::Vec3f::zAxis, math::Vec3f::zAxis);
            gamma = 0;
            forward = math::Math::toCartesian(teta, phi).normalize();
            left = forward.cross(up).normalize();
            target = position + forward;
            viewMatrix.setScale(math::Vec3f(1.f, 1.f, 1.f));
            viewMatrix.setAxis(left, up, forward);
            viewMatrix.setOrigin(position);
            lockTeta = lockPhi = 0;
            viewUpdated = true;
            flipX = false;
            flipY = true;
        }
        void View::setFlipX(bool flipX) {
            this->flipX = flipX;
        }
        void View::setFlipY(bool flipY) {
            this->flipY = flipY;
        }
        bool View::isXFlipped() {
            return flipX;
        }
        bool View::isYFlipped() {
            return flipY;
        }
        float View::getDepth() {
            return depth;
        }
        math::Vec3f View::getSize() {
            return math::Vec3f(viewport.getWidth(), viewport.getHeight(), depth*2);
        }
        math::Vec3f View::getPosition() {
            return math::Vec3f (position.x, position.y, position.z);
        }

        View::View (double width, double height, double fovy, double zNear, double zFar) : viewport(0, 0, zNear, width, height, zFar), depth(zFar) {
            setPerspective(-width * 0.5f, width * 0.5f, -height * 0.5f, height * 0.5f, fovy, width / height, zNear, zFar);
            this->up = math::Vec3f(0, 0, 1);
            position = math::Vec3f (0, 0, 0);
            gamma = 0.f;
            math::Vec3f upToXYPlane(up.x, up.y, 0);
            teta = upToXYPlane.getAngleBetween(math::Vec3f::yAxis, math::Vec3f::yAxis);
            math::Vec3f upToYZPlane(0, up.y, math::Math::abs(up.z));
            phi = upToYZPlane.getAngleBetween(math::Vec3f::zAxis, math::Vec3f::zAxis);
            gamma = 0;
            forward = math::Math::toCartesian(teta, phi).normalize();
            left = forward.cross(up).normalize();
            up = left.cross(forward);
            target = position + forward;
            viewMatrix.setScale(math::Vec3f(1.f, 1.f, 1.f));
            viewMatrix.setAxis(left, up, forward);
            viewMatrix.setOrigin(position);
            lockTeta = lockPhi = 0;
            viewUpdated = true;
            flipX = flipY = false;
        }
        void View::move(float x, float y, float z) {
            math::Vec3f d(x, y, z);
            position += d;
            target = position + forward;
            viewMatrix.setOrigin(position);
            viewUpdated = true;
        }
        physic::BoundingBox View::getFrustum() {
            return projMatrix.getFrustum();
        }
        void View::move (math::Vec3f d, float delta) {
            position += d * delta;
            target = position + forward;
            viewMatrix.setOrigin(position);
            viewUpdated = true;
        }
        void View::setScale(float x, float y, float z) {
            if (x <= 0)
                x = 1;
            if (y <= 0)
                y = 1;
            if (z <= 0)
                z = 1;
            viewMatrix.setScale(math::Vec3f(x, y, z));
        }
        physic::BoundingBox View::getViewVolume() {
            physic::BoundingBox viewVolume(- getSize().x * 0.5f,
                                   - getSize().y * 0.5f,
                                   - getSize().z * 0.5f,
                                   getSize().x,
                                   getSize().y,
                                   getSize().z);
            float angle = math::Math::toRadians(gamma);
            math::Matrix4f transform;
            transform.m11 = (forward.x * forward.x * (1 - math::Math::cosinus(angle)) + math::Math::cosinus(angle));
            transform.m12 = (forward.x * forward.y * (1 - math::Math::cosinus(angle)) - forward.z * math::Math::sinus(angle));
            transform.m13 = (forward.x * forward.z * (1 - math::Math::cosinus(angle)) + forward.y * math::Math::sinus(angle));
            transform.m14 = 0;
            transform.m21 = (forward.y * forward.x * (1 - math::Math::cosinus(angle)) + forward.z * math::Math::sinus(angle));
            transform.m22 = (forward.y * forward.y * (1 - math::Math::cosinus(angle)) + math::Math::cosinus(angle));
            transform.m23 = (forward.y * forward.z * (1 - math::Math::cosinus(angle)) - forward.x * math::Math::sinus(angle));
            transform.m24 = 0;
            transform.m31 = (forward.z * forward.x * (1 - math::Math::cosinus(angle)) - forward.y * math::Math::sinus(angle));
            transform.m32 = (forward.z * forward.y * (1 - math::Math::cosinus(angle)) + forward.x * math::Math::cosinus(angle));
            transform.m33 = (forward.z * forward.z * (1 - math::Math::cosinus(angle)) + math::Math::cosinus(angle));
            transform.m34 = 0;
            transform.m41 = 0;
            transform.m42 = 0;
            transform.m43 = 0;
            transform.m44 = 1;
            math::Vec3f xAxis = left;
            math::Vec3f yAxis = left.cross(forward);
            math::Vec3f zAxis = forward;
            math::Matrix4f matrix4f;
            matrix4f.m11 = xAxis.x;
            matrix4f.m12 = xAxis.y;
            matrix4f.m13 = xAxis.z;
            matrix4f.m14 = -xAxis.dot2(position) / getScale().x;
            matrix4f.m21 = yAxis.x;
            matrix4f.m22 = yAxis.y;
            matrix4f.m23 = yAxis.z;
            matrix4f.m24 = -yAxis.dot2(position) / getScale().y;
            matrix4f.m31 = zAxis.x;
            matrix4f.m32 = zAxis.y;
            matrix4f.m33 = zAxis.z;
            matrix4f.m34 = -zAxis.dot2(position) / getScale().z;
            matrix4f.m41 = 0;
            matrix4f.m42 = 0;
            matrix4f.m43 = 0;
            matrix4f.m44 = 1;
            matrix4f = (transform * matrix4f).inverse();
            std::vector<math::Vec3f> vertices = viewVolume.getVertices();
            for (unsigned int i = 0; i < vertices.size(); i++) {
                //std::cout<<"vertices : "<<vertices[i]<<std::endl;
                vertices[i] = matrix4f * vertices[i];
                //std::cout<<"transformed vertices : "<<vertices[i]<<std::endl;
            }
            std::array<std::array<float, 2>, 3> extends = math::Computer::getExtends(vertices);
            viewVolume = physic::BoundingBox(extends[0][0], extends[1][0], extends[2][0], extends[0][1] - extends[0][0], extends[1][1] - extends[1][0],extends[2][1] - extends[2][0]);
            //std::cout<<"view volume :  "<<viewVolume.getPosition()<<viewVolume.getSize()<<std::endl;
            return viewVolume;
        }
        math::Vec3f View::getScale() {
            return viewMatrix.getScale();
        }
        ViewMatrix View::getViewMatrix() {
            return viewMatrix;
        }
        ProjMatrix View::getProjMatrix () {
            return projMatrix;
        }
        physic::BoundingBox View::getViewport () {
            return viewport;
        }
        void View::reset (physic::BoundingBox rect) {
            viewport = rect;
            viewUpdated = true;
        }
        math::Vec3f View::getLeft() {
            return left;
        }
        math::Vec3f View::getUp() {
            return up;
        }
        math::Vec3f View::getForward() {
            return forward;
        }
        float View::getTeta() {
            return teta;
        }
        float View::getPhi() {
            return phi;
        }
        void View::computeVectorsFromAngles() {
            forward = math::Math::toCartesian(math::Math::toRadians(teta), math::Math::toRadians(phi));
            left = forward.cross(up).normalize();
            target = position + forward;
        }
        void View::lookAt(float x, float y, float z) {
            target = math::Vec3f(x, y, z);
            forward = target - position;
            forward = forward.normalize();
            forward.z = -forward.z;
            if (math::Math::abs(forward.x) < EPSILON && math::Math::abs(forward.y) < EPSILON) {
                if (forward.z > 0)
                    up = math::Vec3f::yAxis;
                else
                    up = -math::Vec3f::yAxis;
            } else {
                up = math::Vec3f::zAxis;
            }
            left = forward.cross(up).normalize();
        }
        void View::rotate(float teta, float phi) {
            if (lockPhi != 0 && phi > lockPhi)
                phi = lockPhi;
            if (lockPhi != 0 &&  phi < -lockPhi)
                phi = -lockPhi;
            this->phi = phi;
            if (lockTeta != 0 &&  teta > lockTeta)
                teta = lockTeta;
            if (lockTeta != 0 &&  teta < -lockTeta)
                teta = -lockTeta;
            this->teta = teta;
            computeVectorsFromAngles();
            viewMatrix.setAxis(left, up, forward);
            viewUpdated = true;
        }
        void View::rotate(float gamma) {
            this->gamma = gamma;
            viewMatrix.setRotation(gamma);
            viewUpdated = true;
        }
        void View::setPerspective (double left, double right, double bottom, double top, double front, double back) {
            projMatrix.setGlOrthoMatrix(left, right, bottom, top, front, back);
            viewUpdated = true;
            ortho = true;
        }
        void View::setPerspective(double left, double right, double bottom, double top, double fovy, double aspect, double front, double back) {
            double tangent = math::Math::tang(math::Math::toRadians(fovy) * 0.5f);   // tangent of half fovY
            double hheight = (front * tangent);          // half height of near plane
            double hwidth = (hheight * aspect);
            projMatrix.setGlPerspectiveMatrix(-hwidth, hwidth, -hheight, hheight, front, back);
            viewUpdated = true;
            ortho = false;
        }
        bool View::isOrtho() {
            return ortho;
        }
        void View::setConstrains (float lockTeta, float lockPhi) {
            this->lockTeta = lockTeta;
            this->lockPhi = lockPhi;
        }
        void View::setCenter(math::Vec3f center) {
            position = center;
            viewMatrix.setOrigin(position);
            viewUpdated = true;
        }
        float View::getGamma() {
            return gamma;
        }
        void View::updated() {
            viewUpdated = false;
        }
    }
}

