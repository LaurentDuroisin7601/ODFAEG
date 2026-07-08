#include <boost/preprocessor.hpp>
/**\fn
* \brief This is an helper function like macro which register a derived type in the dynamic factory.
* \param ID : an ID which is associate to a derived type.
* \param BASE : the base type of the derived class.
* \param DERIVED : the derived type of the derived class.
*/
#define REGISTER_TYPE(ID, BASE, DERIVED, PARAMS, ARGS) \
{ \
    DERIVED *derived##ID = nullptr; \
    odfaeg::core::Allocator<BASE> allocator##ID; \                      
    BASE*(odfaeg::core::Allocator<BASE>::*f##ID)(DERIVED*, \
    BOOST_PP_SEQ_ENUM(PARAMS)) = \
    &odfaeg::core::Allocator<BASE>::allocate<DERIVED, \
    BOOST_PP_SEQ_ENUM(PARAMS)>;                                         \
    \
    odfaeg::core::FastDelegate<BASE*> allocatorDelegate##ID(\
        f##ID, &allocator##ID, derived##ID, BOOST_PP_SEQ_ENUM(ARGS));       \
    \
    odfaeg::core::BaseFactory<BASE>::register_type(\
        typeid(DERIVED).name(), allocatorDelegate##ID);                     \
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

#define REGISTER_FUNC(ID, funcName, SID, BASE, TYPES_SEQ) \
{ \
    using DERIVED = BOOST_PP_SEQ_HEAD(TYPES_SEQ); \
    using PARAM_TYPES = BOOST_PP_SEQ_TAIL(TYPES_SEQ); \
    void(DERIVED::*f##ID##funcName##SID)(BOOST_PP_SEQ_ENUM(PARAM_TYPES)) = \
        &DERIVED::vt##funcName; \
    odfaeg::core::FastDelegate<void> delegate##ID##funcName##SID( \
        f##ID##funcName##SID, \
        MAKE_ARG_LIST(TYPES_SEQ) \
    ); \
\
    odfaeg::core::BaseFactory<BASE>::register_function( \
        typeid(DERIVED).name(), \
        #funcName, \
        #SID, \
        delegate##ID##funcName##SID \
    ); \
}
#define CAT(a,b) a##b
#define EXPAND(a,b) CAT(a,b)
#define OARCH(ARCH) odfaeg::core::EXPAND(O, ARCH)
#define IARCH(ARCH) odfaeg::core::EXPAND(I, ARCH)
#define EXPORT_CLASS_GUID(ID, BASE, DERIVED, ARCHIVE_TYPE, PARAMS, ARGS) \
{ \
    REGISTER_TYPE_(ID, BASE, DERIVED, PARAMS, ARGS); \    
    REGISTER_FUNC(ID, serialize, OARCH(ARCHIVE_TYPE), BASE, (DERIVED)(OARCH(ARCHIVE_TYPE)&)); \
    REGISTER_FUNC(ID, serialize, IARCH(ARCHIVE_TYPE), BASE, (DERIVED)(IARCH(ARCHIVE_TYPE)&)); \
}
