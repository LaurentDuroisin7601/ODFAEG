#include "../../../include/odfaeg/Core/namespace.hpp"
namespace odfaeg {
    namespace core {
        Namespace::Namespace(std::string name) {
            this->name = name;
        }
        void Namespace::addClass(Class classs) {
            classes.push_back(classs);
        }
        std::vector<Class> Namespace::getClasses() {
            return classes;
        }
        std::string Namespace::getName() {
            return name;
        }
    }
}
