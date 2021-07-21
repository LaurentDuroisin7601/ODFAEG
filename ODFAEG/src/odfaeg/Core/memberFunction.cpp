#include "../../../include/odfaeg/Core/memberFunction.hpp"
namespace odfaeg {
    namespace core {
        MemberFunction::MemberFunction(std::string returnType, std::string name) : m_name(name), m_returnType(returnType) {
        }
        void MemberFunction::addArgType(std::string argType) {
            m_argsTypes.push_back(argType);
        }
        void MemberFunction::addArgName(std::string argName) {
            m_argsNames.push_back(argName);
        }
        std::string MemberFunction::getReturnType() {
            return m_returnType;
        }
        std::string MemberFunction::getName() {
            return m_name;
        }
        std::vector<std::string> MemberFunction::getArgsTypes() {
            return m_argsTypes;
        }
        std::vector<std::string> MemberFunction::getArgsNames() {
            return m_argsNames;
        }
    }
}
