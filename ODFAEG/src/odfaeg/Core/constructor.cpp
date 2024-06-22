#include "../../../include/odfaeg/Core/constructor.hpp"
namespace odfaeg {
    namespace core {
        Constructor::Constructor(std::string name) : name(name) {
        }
        void Constructor::addArgType(std::string argType) {
            argsTypes.push_back(argType);
        }
        void Constructor::addArgName(std::string argName) {
            argsNames.push_back(argName);
        }
        std::vector<std::string> Constructor::getArgsNames() {
            return argsNames;
        }
        std::vector<std::string> Constructor::getArgsTypes() {
            return argsTypes;
        }
        std::string Constructor::getName() {
            return name;
        }
    }
}
