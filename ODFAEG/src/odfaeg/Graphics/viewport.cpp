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
        void ViewportMatrix::update() {
            if (viewportUpdated) {
                viewport[0][0] = std::abs(size.x()) * 0.5f;
                viewport[0][1] = 0.f;
                viewport[0][2] = 0.f;
                viewport[0][3] = std::abs(size.x()) * 0.5f + position.x();
                viewport[1][0] = 0.f;
                viewport[1][1] = std::abs(size.y()) * 0.5f;
                viewport[1][2] = 0.f;
                viewport[1][3] = std::abs(size.y()) * 0.5f + position.y();
                viewport[2][0] = 0.f;
                viewport[2][1] = 0.f;
                #ifndef VULKAN
                viewport[2][2] = std::abs(size.z()) * 0.5f;
                viewport[2][3] = std::abs(size.z()) * 0.5f + position.z();
                #else
                viewport[2][2] = 1.0f;
                viewport[2][3] = 0.0f;
                #endif
                viewport[3][0] = std::min(position.x(), size.x());
                viewport[3][1] = std::min(position.y(), size.y());
                viewport[3][2] = std::min(position.z(), size.z());
                viewport[3][3] = 1.f;
                viewportUpdated = false;
            }
        }
        math::Vec4f ViewportMatrix::toViewportCoordinates(math::Vec4f vec) {
            vec[3] = 1;
            update();
            return viewport * vec;
        }
        math::Vec4f ViewportMatrix::toNormalizedCoordinates(math::Vec4f vec) {
            vec[3] = 1;
            update();
            ////////std::cout<<"matrix : "<<viewport<<std::endl;
            return viewport.inverse() * vec;
        }
        math::Matrix4f ViewportMatrix::getMatrix() {
            update();
            return viewport;
        }
    }
}
