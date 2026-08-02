#include <cmath>
#include <algorithm>
#include "../Math/vec.hpp"
#include "../Math/matrix.hpp"
namespace odfaeg {
    namespace graphic {
        export class ViewportMatrix {
        public:
            ViewportMatrix();
            void setViewport(math::Vec3f position, math::Vec3f size);
            void setScale(math::Vec3f scale);
            math::Vec4f toViewportCoordinates(math::Vec4f vec);
            math::Vec4f toNormalizedCoordinates(math::Vec4f vec);
            math::Matrix4f getMatrix();
            void update();
        private:
            math::Matrix4f viewport;
            math::Vec3f position;
            math::Vec3f size;
            math::Vec3f scale;
            bool viewportUpdated;
        };
    }
}
#include "viewportMatrix.inl"