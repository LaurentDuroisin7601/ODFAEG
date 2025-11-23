#ifndef MEMBER_VARIABLE_HPP
#define MEMBER_VARIABLE_HPP
#include <string>
namespace odfaeg {
    namespace core {
        class MemberVariable {
        public :
            void setVarType(std::string varType);
            std::string getVarType();
            void setVarName(std::string varName);
            std::string getVarName();
        private :
            std::string varType;
            std::string varName;
        };
    }
}
#endif // MEMBER_VARIABLE_HPP
