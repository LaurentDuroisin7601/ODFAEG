#include "../../../include/odfaeg/Graphics/viewportMatrix.h"
namespace odfaeg {
    namespace graphic {
        ViewportMatrix::ViewportMatrix() {
            viewportUpdated = true;
            scale = math::Vec3f(1.f, 1.f, 1.f);
            position = math::Vec3f(-1.f, -1.f, -1.f);
            size = math::Vec3f(2.f, 2.f, 2.f);
        }
        void ViewportMatrix::setViewport(math::Vec3f position, math::Vec3f size) {
            this->position = position;
            this->size = size;
            viewportUpdated = true;
        }
        void ViewportMatrix::setScale(math::Vec3f scale) {
            this->scale = scale;
            viewportUpdated = true;
        }
        math::Vec3f ViewportMatrix::toViewportCoordinates(math::Vec3f vec) {
            if (viewportUpdated) {
                viewport.m11 = std::abs(size.x) * 0.5f;
                viewport.m12 = 0.f;
                viewport.m13 = 0.f;
                viewport.m14 = std::abs(size.x) * 0.5f + position.x;
                viewport.m21 = 0.f;
                viewport.m22 = std::abs(size.y) * 0.5f;
                viewport.m23 = 0.f;
                viewport.m24 = std::abs(size.y) * 0.5f + position.y;
                viewport.m31 = 0.f;
                viewport.m32 = 0.f;
                viewport.m33 = std::abs(size.z) * 0.5f;
                viewport.m34 = std::abs(size.z) * 0.5f + position.z;
                viewport.m41 = std::min(position.x, size.x);
                viewport.m42 = std::min(position.y, size.y);
                viewport.m43 = std::min(position.y, size.z);
                viewport.m44 = 1.f;
                viewportUpdated = false;
            }
            return viewport * vec;
        }
        math::Vec3f ViewportMatrix::toNormalizedCoordinates(math::Vec3f vec) {
            if (viewportUpdated) {
                viewport.m11 = std::abs(size.x) * 0.5f;
                viewport.m12 = 0.f;
                viewport.m13 = 0.f;
                viewport.m14 = std::abs(size.x) * 0.5f + position.x;
                viewport.m21 = 0.f;
                viewport.m22 = std::abs(size.y) * 0.5f;
                viewport.m23 = 0.f;
                viewport.m24 = std::abs(size.y) * 0.5f + position.y;
                viewport.m31 = 0.f;
                viewport.m32 = 0.f;
                viewport.m33 = std::abs(size.z) * 0.5f;
                viewport.m34 = std::abs(size.z) * 0.5f + position.z;
                viewport.m41 = std::min(position.x, size.x);
                viewport.m42 = std::min(position.y, size.y);
                viewport.m43 = std::min(position.y, size.z);
                viewport.m44 = 1.f;
                viewportUpdated = false;
            }
            //std::cout<<"matrix : "<<viewport<<std::endl;
            return viewport.inverse() * vec;
        }
        math::Matrix4f ViewportMatrix::getMatrix() {
            if (viewportUpdated) {
                viewport.m11 = std::abs(size.x) * 0.5f;
                viewport.m12 = 0.f;
                viewport.m13 = 0.f;
                viewport.m14 = std::abs(size.x) * 0.5f + position.x;
                viewport.m21 = 0.f;
                viewport.m22 = std::abs(size.y) * 0.5f;
                viewport.m23 = 0.f;
                viewport.m24 = std::abs(size.y) * 0.5f + position.y;
                viewport.m31 = 0.f;
                viewport.m32 = 0.f;
                viewport.m33 = std::abs(size.z) * 0.5f;
                viewport.m34 = std::abs(size.z) * 0.5f + position.z;
                viewport.m41 = std::min(position.x, size.x);
                viewport.m42 = std::min(position.y, size.y);
                viewport.m43 = std::min(position.y, size.z);
                viewport.m44 = 1.f;
                viewportUpdated = false;
            }
            return viewport;
        }
    }
}
