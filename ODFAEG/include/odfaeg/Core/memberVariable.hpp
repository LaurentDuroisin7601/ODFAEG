#ifndef MEMBER_VARIABLE_HPP
#define MEMBER_VARIABLE_HPP
#include <string>
#include <clang-c/Index.h>
namespace odfaeg {
    namespace core {
        class MemberVariable {
        public :
            void setVarType(std::string varType);
            std::string getVarType();
            void setVarName(std::string varName);
            std::string getVarName();
            CXTranslationUnit tu;
        private :
            std::string varType;
            std::string varName;
        };
    }
}
#endif // MEMBER_VARIABLE_HPP
