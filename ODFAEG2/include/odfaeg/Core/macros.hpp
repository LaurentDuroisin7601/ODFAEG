#include <boost/preprocessor.hpp>
/**\fn
* \brief This is an helper function like macro which register a derived type in the dynamic factory.
* \param ID : an ID which is associate to a derived type.
* \param BASE : the base type of the derived class.
* \param DERIVED : the derived type of the derived class.
*/

#define REGISTER_TYPE(ID, BASE, DERIVED)                                \
{                                                                       \
    struct Wrapper##ID##funcName##SID {                                       \
        static B* call() {                        \
            return new Derived();               \
        }                                                                     \
    };                                                                        \
                                                                            \
    odfaeg::core::FastDelegate<void> delegate##ID##funcName##SID(             \
        &Wrapper##ID##funcName##SID::call                                      \
    );                                                                         \
                                                                            \
    odfaeg::core::BaseFactory<BASE>::register_function(                       \
        typeid(DERIVED).name(),                                               \
        #funcName,                                                            \
        #SID,                                                                 \
        delegate##ID##funcName##SID                                           \
    );                                                          \
}                    \
/**fn
* \brief This is an helper function like macro which register a function in the dynamic factory.
* \param ID : an ID which is associate to a derived type.
* \param funcName : the name of the derived class member's function to register.
* \param SID : an ID associated to the argument list of the member's function to register.
* \param BASE : the base type of the derived class.
* \param DERIVED : the derived type of the derived class.
* \param SIGNATURE : the signature of the function to register.
*/
#define REGISTER_FUNC(ID, funcName, SID, BASE, DERIVED, ARCHIVE)        \
{                                                                       \
    /* wrapper statique compatible Clang/GCC/MSVC */                          \
    struct Wrapper##ID##funcName##SID {                                       \
        static void call(BASE* baseObj, ARCHIVE& ar) {                        \
            static_cast<DERIVED*>(baseObj)->vt##funcName(ar);                 \
        }                                                                     \
    };                                                                        \
                                                                            \
    odfaeg::core::FastDelegate<void> delegate##ID##funcName##SID(             \
        &Wrapper##ID##funcName##SID::call                                      \
    );                                                                         \
                                                                            \
    odfaeg::core::BaseFactory<BASE>::register_function(                       \
        typeid(DERIVED).name(),                                               \
        #funcName,                                                            \
        #SID,                                                                 \
        delegate##ID##funcName##SID                                           \
    );                                                                     \
}
#define CAT(a,b) a##b 
#define EXPAND(a,b) CAT(a,b) 
#define OARCH(ARCH) EXPAND(O, ARCH) 
#define IARCH(ARCH) EXPAND(I, ARCH) 
#define EXPORT_CLASS_GUID(ID, BASE, DERIVED, ARCHIVE_TYPE) \
REGISTER_TYPE(ID, BASE, DERIVED);                                   \
REGISTER_FUNC(ID, serialize, ARCHIVE_TYPE, BASE, DERIVED,    \
            ARCHIVE_TYPE);                   \
/*REGISTER_FUNC(ID, serialize, IARCH(ARCHIVE_TYPE), BASE, DERIVED,    \
            ARCHIVE_TYPE); */
    
    

 
