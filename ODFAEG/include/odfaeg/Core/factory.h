//fichier fabrique.h
#ifndef ODFAEG_FACTORY_HPP
#define ODFAEG_FACTORY_HPP
#include <map>
#include <string>
#include "export.hpp"
#include "fastDelegate.h"
/**\file factory.h
*  \brief This file register types of derived objects
*  and instanciate the polymorphic objects of the right type with the given type id.
*  There are two factories
*  The static factory (The class Factory) which evaluate objects and functions at compile time.
*  The dynamic factory (The class BaseFactory) which evaluate objects and functions at runtime.
*  The dynamic factory avoids to have a reflexion system which is impossible to make in c++ without a lot of macros.
*  \author Duroisin.L
*  \version 1.0
*  \date 1/02/2014
*/
/********************************************************************************************************************/
/**\fn
* \brief This is an helper function like macro which register a derived type in the dynamic factory.
* \param ID : an ID which is associate to a derived type.
* \param BASE : the base type of the derived class.
* \param DERIVED : the derived type of the derived class.
*/
#define REGISTER_TYPE_(ID, BASE, DERIVED, PARAMS, ARGS) \
{ \
DERIVED *derived##ID = nullptr; \
odfaeg::core::Allocator<BASE> allocator##ID; \
BASE*(odfaeg::core::Allocator<BASE>::*f##ID)(DERIVED*, PARAMS) = &odfaeg::core::Allocator<BASE>::allocate<DERIVED, PARAMS>; \
odfaeg::core::FastDelegate<BASE*> allocatorDelegate##ID(f##ID, &allocator##ID, derived##ID, ARGS); \
odfaeg::core::BaseFactory<BASE>::register_type(typeid(DERIVED).name(), allocatorDelegate##ID); \
}
#define REGISTER_TYPE(ID, BASE, DERIVED) \
{ \
DERIVED *derived##ID = nullptr; \
odfaeg::core::Allocator<BASE> allocator##ID; \
BASE*(odfaeg::core::Allocator<BASE>::*f##ID)(DERIVED*) = &odfaeg::core::Allocator<BASE>::allocate<DERIVED>; \
odfaeg::core::FastDelegate<BASE*> allocatorDelegate##ID(f##ID, &allocator##ID, derived##ID); \
odfaeg::core::BaseFactory<BASE>::register_type(typeid(DERIVED).name(), allocatorDelegate##ID); \
}
/**fn
* \brief This is an helper function like macro which register a function in the dynamic factory.
* \param ID : an ID which is associate to a derived type.
* \param funcName : the name of the derived class member's function to register.
* \param SID : an ID associated to the argument list of the member's function to register.
* \param BASE : the base type of the derived class.
* \param DERIVED : the derived type of the derived class.
* \param SIGNATURE : the signature of the function to register.
*/
#define REGISTER_FUNC(ID, funcName, SID, BASE, DERIVED, SIGNATURE, ...) \
{ \
void(DERIVED::*f##ID##funcName##SID)SIGNATURE = &DERIVED::vt##funcName; \
odfaeg::core::FastDelegate<void> delegate##ID##funcName##SID (f##ID##funcName##SID, __VA_ARGS__); \
odfaeg::core::BaseFactory<BASE>::register_function(typeid(DERIVED).name(), #funcName, #SID, delegate##ID##funcName##SID); \
}
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace core {
        /**\class Factory
        *  \brief This class register types of derived objects
        *  and instanciate the polymorphic objects of the right type with the given type id.
        *  This is a static factory, it means that the id is defined at compile time.
        *  \author Duroisin.L
        *  \version 1.0
        *  \date 1/02/2014
        */
        template <class O, class K = std::string>
        class ODFAEG_CORE_API Factory {
        private :
             static std::map<K, O*> m_map;
        public:
             static void Register (K key, O* object);
             O* Create (const K& key);
        };
        /**\class Allocator
        *  \brief this struct allocate an object of a derived type and return a pointer of the base type.
        *  \param B : the base type.
        */
        template <typename B>
        struct Allocator {

           /**\fn allocate(D*)
           *  \brief this function allocates an object of a derived type and return a pointer of the base type.
           *  \param D : the derived type.
           *  \return B* : a pointer to the base type.
           */
           template <typename D, class... Args>
           B* allocate(D* d, Args&&... args) {
                return new D(std::forward<Args>(args)...);
           }
        };
        /**\class BaseFactory
        *  \brief This class register types of derived objects
        *  and instanciate the polymorphic objects of the right type with the given type id.
        *  This is a dynamic factory, it means that the id is defined at runtime time with RTTI infos.
        *  So you must provide the id with typeid(object).name().
        *  \author Duroisin.L
        *  \version 1.0
        *  \date 1/02/2014
        */
        template <typename B>
        class BaseFactory {
            public :
            /**
            * \fn void register_type (std::string typeName, FastDelegate<B*> allocatorDelegate)
            * \brief register a type into a factory, if the type isn't already registered.
            * \param std::string typeName : the name of the type to register.
            * \param a callback function to an allocator, to return a pointer of the base class which point
            * to the derived object.
            */
            static void register_type(std::string typeName, FastDelegate<B*> allocatorDelegate) {
                typename std::map<std::string, FastDelegate<B*>>::iterator it = types.find(typeName);
                if (it == types.end()) {
                    types[typeName] = allocatorDelegate;
                }
            }
            /** \fn void register_function(std::string typeName, std::string funcName, std::string funcArgs, FastDelegate<void> delegate)
            *   \brief register a member function of a class type into the factory.
            *   \param std::string typeName : the type name of the class containing the member function to register.
            *   \param std::string funcName : the name of the function to register.
            *   \param std::string funcArgs : the name of the types of the member function's argument list to register.
            *   \param FastDelegate<void> delegate : a callback function of the registered member function. (only funcions returing void can be registered!)
            */
            static void register_function(std::string typeName, std::string funcName, std::string funcArgs, FastDelegate<void> delegate) {
                typename std::map<std::string, FastDelegate<void>>::iterator it = functions.find(typeName+funcName+funcArgs);
                if (it == functions.end()) {
                    //std::cout<<"register function : "<<typeName+funcName+funcArgs<<std::endl;
                    functions[typeName+funcName+funcArgs] = delegate;
                }
            }
            /** \fn void callFunction(std::string typeName, std::string funcName, std:string fincArgs, A&&... args)
            *   \brief call a registered function of the factory, throw an error if the function isn't registered.
            *   \param std::string typeName : the type name of the class containing the member function.
            *   \param std::string funcName : the name of the member function.
            *   \param std::string funcArgs : the name of the types of the member function's argument list to call.
            *   \param A&& args.... : the value of the arguments to pass to the callback's member function. (object + arguments)
            */
            template <typename... A>
            static void callFunction(std::string typeName, std::string funcName, std::string funcArgs, A&&... args) {
                typename std::map<std::string, FastDelegate<void>>::iterator it = functions.find(typeName+funcName+funcArgs);
                //std::cout<<"call function : "<<typeName+","+funcName+","+funcArgs<<std::endl;
                if (it != functions.end()) {
                    it->second.setParams(std::forward<A>(args)...);
                    (it->second)();
                } else {
                    throw std::runtime_error("Unregistred function exception!");
                }
            }
            /** \fn B* create (std::string typeName)
            *   \brief return a pointer of the base class of an object which point to an object of a derived type.
            * this function call a callback function to an allocator function to allocate the object.
            *   \param the typeName of the type to allocate.
            *   \return B* a pointer of a base class which'll point to the allocated object.
            */
            template <class... Args>
            static B* create (std::string typeName, Args&&... args) {
                typename std::map<std::string, FastDelegate<B*>>::iterator it = types.find(typeName);
                if (it != types.end()) {
                    return (it->second)();
                }
                throw std::runtime_error("Unregistred type exception!"+typeName);
            }
            /** \fn std::string getTypeName (B* type)
            *   \brief return the type name of a base object.
            *   \param B* type : a pointer to the type to check the dynamic type name.
            *   \return the dynamic type name of the passed object*/
            static std::string getTypeName (B* type) {
                typename std::map<std::string, FastDelegate<B*>>::iterator it = types.find(typeid(*type).name());
                if (it != types.end())
                    return it->first;
				return "";
            }
            private :
            static std::map<std::string, FastDelegate<B*>> types; /**> An std::map which store the typeName and a callback's function to an allocator of the registered types*/
            static std::map<std::string, FastDelegate<void>> functions; /**> An std::map which store the signature and a callback's function to the registered member's functions.*/
        };
        template <typename B>
        std::map<std::string, FastDelegate<B*>> BaseFactory<B>::types = std::map<std::string, FastDelegate<B*>>();
        template <typename B>
        std::map<std::string, FastDelegate<void>> BaseFactory<B>::functions = std::map<std::string, FastDelegate<void>>();
    }
}
#endif
