#ifndef ODFAEG_VIEWPORT_MATRIX
#define ODFAEG_VIEWPORT_MATRIX
#include "../Math/matrix4.h"
namespace odfaeg {
    namespace graphic {
        class ViewportMatrix {
        public :
            ViewportMatrix();
            void setViewport(math::Vec3f position, math::Vec3f size);
            void setScale(math::Vec3f scale);
            math::Vec3f toViewportCoordinates(math::Vec3f vec);
            math::Vec3f toNormalizedCoordinates(math::Vec3f vec);
            math::Matrix4f getMatrix();
        private :
            math::Matrix4f viewport;
            math::Vec3f position;
            math::Vec3f size;
            math::Vec3f scale;
            bool viewportUpdated;
        };
    }
}
#endif // ODFAEG_VIEWPORT_MATRIX
