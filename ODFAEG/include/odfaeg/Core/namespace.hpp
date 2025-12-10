#ifndef NAMESPACE_HPP
#define NAMESPACE_HPP
#include "class.hpp"
namespace odfaeg {
    namespace core {
        class Namespace {
        public :
            Namespace(std::string name);
            void addClass(Class classs);
            std::vector<Class> getClasses();
            std::string getName();
        private :
            std::vector<Class> classes;
            std::string name;
        };
    }
}
#endif // NAMESPCACE_HPP
