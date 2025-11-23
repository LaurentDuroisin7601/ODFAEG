#include "../../../include/odfaeg/Core/memberVariable.hpp"
namespace odfaeg {
    namespace core {
        void MemberVariable::setVarType(std::string varType) {
            this->varType = varType;
        }
        void MemberVariable::setVarName(std::string varName) {
            this->varName = varName;
        }
        std::string MemberVariable::getVarType() {
            return varType;
        }
        std::string MemberVariable::getVarName() {
            return varName;
        }
    }
}
