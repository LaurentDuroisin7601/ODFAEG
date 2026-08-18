#include <iostream>
/**\fn
* \brief This is an helper function like macro which register a derived type in the dynamic factory.
* \param ID : an ID which is associate to a derived type.
* \param BASE : the base type of the derived class.
* \param DERIVED : the derived type of the derived class.
*/

#define REGISTER_TYPE(ID, BASE, DERIVED)                                \
{                                                                       \
    odfaeg::core::FastDelegate<BASE*> allocatorDelegate##ID(            \
        []() -> BASE* {                                                 \
            return new DERIVED();                                       \
        }                                                             \
    );                                                                  \
                                                                      \
    odfaeg::core::BaseFactory<BASE>::register_type(                     \
        typeid(DERIVED).name(),                                         \
        allocatorDelegate##ID                                           \
    );                                                                  \
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
    odfaeg::core::FastDelegate<void> delegate##ID##funcName##SID(      \
        [](BASE* baseObj, ARCHIVE* ar) {                                \
            auto* d = static_cast<DERIVED*>(baseObj);   \
            std::cout<<"ar : "<<&ar<<" d : "<<d<<std::endl;                \
            d->vt##funcName(*ar);                                       \
        }, ph<0, BASE*>{}, ph<1, ARCHIVE*>{}                                                               \
    );                                                                  \
                                                                        \
    odfaeg::core::BaseFactory<BASE>::register_function(                \
        typeid(DERIVED).name(),                                         \
        #funcName,                                                      \
        #SID,                                                           \
        delegate##ID##funcName##SID                                     \
    );                                                                  \
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
            IARCH(ARCHIVE_TYPE));*/
    

 
