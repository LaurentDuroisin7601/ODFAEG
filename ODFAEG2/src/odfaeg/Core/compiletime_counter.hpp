#ifndef ODFAEG_COMPILETIME_COUNTER
#define ODFAEG_COMPILETIME_COUNTER
#include <iostream>
namespace odfaeg {
    namespace core {
        template <auto, auto>
        struct ADLType {
            friend consteval auto ADLFunc(ADLType, auto...);
        };

        template <auto InstanceIdentifier, auto x>
        struct InjectDefinitionForADLFunc {
            friend consteval auto ADLFunc(ADLType<InstanceIdentifier, x>, auto...) {}
        };

        template<auto InstanceIdentifier, auto x = 0, auto = []{}>
        constexpr auto ExtractThenUpdateCurrentState()->decltype(x) {
            if constexpr (requires { ADLFunc(ADLType<InstanceIdentifier, x>{}); }) {
                //std::cout<<"requires true"<<std::endl;
                return ExtractThenUpdateCurrentState<InstanceIdentifier, x + 1>();
            } else {
                
                //std::cout<<"requires false"<<std::endl;
                InjectDefinitionForADLFunc<InstanceIdentifier, x>{};
                //Fonctionne pas.
                /*constexpr auto ID = InstanceIdentifier{};
                static_assert(
                    requires {
                        ADLFunc(ADLType<ID, 0>{});
                    }
                );*/
            }
            return x;
        }        
        template<auto InstanceIdentifier, auto x = 0, auto = []{}>
        constexpr auto ExtractCurrentState()->decltype(x) {
            return x;
        }
        template<auto InstanceIdentifier, auto x = 0, auto = []{}>
        constexpr auto ResetCurrentState()->decltype(x) {
            return 0;
        }

        template <auto InstanceIdentifier = []{}>
        struct Compteur {
            template <auto x = ExtractThenUpdateCurrentState<InstanceIdentifier>()>
            consteval auto next() const { return x; } 
            template <auto x = ExtractCurrentState<InstanceIdentifier>()>
            consteval auto value() const { return x; }  
            template <auto x = ResetCurrentState<InstanceIdentifier>()>
            void reset() const {

            }
        };
    }
}
#endif