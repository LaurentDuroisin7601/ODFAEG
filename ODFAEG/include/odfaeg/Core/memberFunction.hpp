#ifndef ODFAEG_CORE_MEMBER_FUNCTION_HPP
#define ODFAEG_CORE_MEMBER_FUNCTION_HPP
#include <vector>
#include <string>
#include "export.hpp"
#include <clang-c/Index.h>
namespace odfaeg {
    namespace core {
        class ODFAEG_CORE_API MemberFunction {
        public :
            MemberFunction(std::string returnType, std::string name);
            void addArgType(std::string argType);
            void addArgName(std::string argName);
            std::string getReturnType();
            std::string getName();
            std::vector<std::string> getArgsTypes();
            std::vector<std::string> getArgsNames();
            CXTranslationUnit tu;
            std::pair<unsigned int, unsigned int> location;
        private :
            std::string m_name;
            std::string m_returnType;
            std::vector<std::string> m_argsTypes;
            std::vector<std::string> m_argsNames;
        };
    }
}
#endif // ODFAEG_CORE_MEMBER_FUNCTION_HPP
