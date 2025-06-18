#ifndef ODFAEG_VIEWPORT_MATRIX
#define ODFAEG_VIEWPORT_MATRIX
#include "../Math/matrix4.h"
#include "export.hpp"
namespace odfaeg {
    namespace graphic {
        class ODFAEG_GRAPHICS_API ViewportMatrix {
        public :
            ViewportMatrix();
            void setViewport(math::Vec3f position, math::Vec3f size);
            void setScale(math::Vec3f scale);
            math::Vec4f toViewportCoordinates(math::Vec4f vec);
            math::Vec4f toNormalizedCoordinates(math::Vec4f vec);
            math::Matrix4f getMatrix();
            void update();
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
