#ifndef ODFAEG_SERIALIZATION_HPP
#define ODFAEG_SERIALIZATION_HPP
#include <iostream>
#include <sstream>
#include <typeinfo>
#include <tuple>
#include <map>
#include "erreur.h"
#include "archive.h"
#include "factory.h"

/** \fn EXPORT_CLASS_GUID(ID, BASE, DERIVED)
*   \brief Helper macro which export classes to serialize member's variables of polymorphic objects.
*   \param ID : an unique ID which identify the exportation.
*   \param BASE : the type of the base class to export.
*   \param DERIVED : the type of the derived class to export.
*   Exemple (for multiple inheritance) :
*   EXPORT_CLASS_GUID(B1D, BASE1, DERIVED)
*   EXPORT_CLASS_GUID(B2D, BASE2, DERIVED)
*   etc...
*   Exemple (for single inheritance)
*   EXPORT_CLASS_GUID(BD1, BASE, DERIVED1)
*   EXPORT_CLASS_GUID(BD2, BASE, DERIVED2)
*   etc...
*   Abstract derived classes cannot be exported.
*/
#define VA_LIST(...) __VA_ARGS__
#define EXPORT_CLASS_GUID_(ID, BASE, DERIVED, PARAMS, ARGS) \
{ \
REGISTER_TYPE_(ID, BASE, DERIVED, PARAMS , ARGS) \
std::ostringstream oss##ID; \
std::istringstream iss##ID; \
odfaeg::core::OTextArchive outa##ID(oss##ID); \
odfaeg::core::ITextArchive ina##ID(iss##ID); \
BASE* derived##ID = nullptr; \
REGISTER_FUNC(ID, serialize, OTextArchive, BASE, DERIVED, (odfaeg::core::OTextArchive&), derived##ID, std::ref(outa##ID)) \
REGISTER_FUNC(ID, serialize, ITextArchive, BASE, DERIVED, (odfaeg::core::ITextArchive&), derived##ID, std::ref(ina##ID)) \
}
#define EXPORT_CLASS_GUID(ID, BASE, DERIVED) \
{ \
REGISTER_TYPE(ID, BASE, DERIVED) \
std::ostringstream oss##ID; \
std::istringstream iss##ID; \
odfaeg::core::OTextArchive outa##ID(oss##ID); \
odfaeg::core::ITextArchive ina##ID(iss##ID); \
BASE* derived##ID = nullptr; \
REGISTER_FUNC(ID, serialize, OTextArchive, BASE, DERIVED, (odfaeg::core::OTextArchive&), derived##ID, std::ref(outa##ID)) \
REGISTER_FUNC(ID, serialize, ITextArchive, BASE, DERIVED, (odfaeg::core::ITextArchive&), derived##ID, std::ref(ina##ID)) \
}
/**
 *\namespace odfaeg
 * the namespace of the Opensource Development Framework Adapted for Every Games.
 */
namespace odfaeg {
    namespace core {
        /**
        * \file serialization.h
        * \class Serializer
        * \param the type of the base class.
        * \brief Serialize member's attribute of a class.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        template <class B>
        class Serializer : public BaseFactory<B> {
            public :
            /** \fn Serializer()
            *   \brief constructor
            */
            Serializer() : BaseFactory<B>() {
                baseObject = nullptr;
            }
            /** \fn void setObject(B* baseObject)
            *   \brief set to polymoprhic object to serialize.
            *   \param B* baseObject : the polymoprhic object to serialize.
            */
            void setObject(B* baseObject) {
                this->baseObject = baseObject;
            }
            /** \fn void serialize(std::string funcName, std::string funcArgs, Archive & ar)
            *   \brief call the member's function of the dynamic type of the polymoprhic object to serialize its member's attributes into the given archive.
            *   \param std::string funName : the name of the function which'll serialize the datas.
            *   \param std::string funcArgs :the signature id of the function which'll serialize the datas.
            *   \param Archive & ar : the archive where the data'll be serialized.
            */
            template <typename Archive>
            void serialize(std::string funcName, std::string funcArgs, Archive & ar) {
                BaseFactory<B>::callFunction(typeid(*baseObject).name(), funcName, funcArgs, baseObject, std::ref(ar));
            }
            /**\fn B* sallocate (std::string typeName)
            *  \brief std::string typeName : allocate an object of the given dynamic type name.
            *  \param std::string typeName : the name of the dynamic type.
            *  \return B* : a pointer to the allocated object.
            */
            B* sallocate(std::string typeName) {
                return BaseFactory<B>::create(typeName);
            }
            /**\fn std::string getTypeName()
            *  \brief get the name of the dynamic type of the polymorphic object.
            *  \return std::string : the dynamic type of the object.
            */
            std::string getTypeName() {
                return BaseFactory<B>::getTypeName(baseObject);
            }
            private :
            B* baseObject; /** B* baseObject : a pointer to a polymorphic object.*/
        };
        /**
        * \file serialization.h
        * \class Registered
        * \param the type of the base class.
        * \brief Base class which keep a key which know how to serialize polymorphic objects.
        * Every base class of polymoprhic object should inherits from this class.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        template <typename Base>
        struct Registered {
            /**
            * \file serialization.h
            * \class Key
            * \brief sub class which call the functions to serialize a polymorphic object througth the key.
            * So, as the polymorphic object inherits from this class, the archive can call the functions to allocate and
            * to serialize the polymorphic object by only knowing the base type of the polymoprhic object at compile time
            * like this : object->key.serialize(*this), Registered<Base>::allocate(typeid(*base).name()).
            * An object is considered as polymoprhic if it contains a typedef named KEYTYPE. (It happens only if its base class inherits from Registered<Base>)
            * \author Duroisin.L
            * \version 1.0
            * \date 1/02/2014
            */
            struct Key:
            public Serializer<Base> {
            public:
                /**\fn Key()
                *  \brief constructor
                */
                Key() : Serializer<Base>() {}
                /**\fn void register_object (Base* object)
                *  \brief register a polymoprhic object to serialize.
                *  \param Base* object : the polymoprhic object to serialize.
                */
                void register_object (Base* object) {
                     Serializer<Base>::setObject(object);
                }
                Base* allocate_object(std::string typeName) { return Serializer<Base>::sallocate(typeName); }
                /**\fn std::string getTypeName ()
                *  \brief get the dynamic type of the registered object.
                *  \return std::string the dynamic type of the registered object.
                */
                std::string getTypeName () { return Serializer<Base>::getTypeName(); }
                /** \fn void serialize_object (std::string funcName, std::string funcArgs, Archive & ar)
                *   \brief serialize the registered object into the given archive.
                *   \param std::string funName : the name of the function which'll serialize the datas.
                *   \param std::string funcArgs :the signature id of the function which'll serialize the datas.
                *   \param Archive & ar : the archive where the data'll be serialized.
                */
                template <typename Archive>
                void serialize_object (std::string funcName, std::string funcArgs, Archive & ar) {
                    Serializer<Base>::serialize(funcName, funcArgs, ar);
                }
            };
        public:
            typedef Key KEYTYPE; /**> typedef to the type of the key, used for SFINAE.*/
            /** \fn Base* allocate (std::string typeName)
            *   \brief allocate an object of the given dynamic type, return a pointer to the static type.
            *   \param std::string typeName : the name of the dynamic type
            *   \return Base* a pointer to the static type.*/
            static Base* allocate (std::string typeName) {
                static KEYTYPE aKey;
                return aKey.allocate_object(typeName);
            }
            Key key; /**> A key which is registered into the base class of the polymorphic object, it allows the archive to call
            * the functions to serialize the polymoprhic object without knowing its dynamic type.*/
        };
    }
}
#endif // SERIALIZATION
