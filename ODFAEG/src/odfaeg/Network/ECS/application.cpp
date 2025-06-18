#include "../../../../include/odfaeg/Network/ECS/application.hpp"
namespace odfaeg {
    namespace core {
        namespace ecs {
            using namespace sf;
            template <typename A, typename T>
            Clock Application<A, T>::timeClk = Clock();
        }
    }
}
