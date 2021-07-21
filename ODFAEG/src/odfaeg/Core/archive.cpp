#include "../../../include/odfaeg/Core/archive.h"
namespace odfaeg {
    namespace core {
        std::map<std::string, FastDelegate<void*>> ITextArchive::allocators = std::map<std::string, FastDelegate<void*>>();
    }
}
