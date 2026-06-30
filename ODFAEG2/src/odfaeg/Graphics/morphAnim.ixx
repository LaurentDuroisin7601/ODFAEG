module;
#include <vector>
export module odfaeg.graphic.morphAnim;
import odfaeg.graphic.gameObject;
import odfaeg.math.vec;
namespace odfaeg {
    namespace graphic {
        export class MorphAnim : public GameObject {
            public :
            MorphAnim(math::Vec3f position, math::Vec3f size, unsigned int interpLevels=1);
            void addFrame(GameObject* frame);
            std::vector<GameObject*> getFrames();
            unsigned int getIntLevels();
            GameObject* clone();
            private :
            std::vector<GameObject*> frames;
            unsigned int currentFrameIndex, interpLevels;
        };
    }
}
