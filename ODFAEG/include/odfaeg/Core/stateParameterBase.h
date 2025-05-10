#ifndef UPS_State_PARAMETER_BASE
#define UPS_State_PARAMETER_BASE
#include <string>
namespace odfaeg {
    namespace core {
        class ODFAEG_CORE_API StateParameterBase {
            public :

               virtual std::string getName() const = 0;
               virtual void setName(std::string name) = 0;
        };
    }
}
#endif
