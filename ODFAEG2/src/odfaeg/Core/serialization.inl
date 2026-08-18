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
        /** \fn Serializer()
        *   \brief constructor
        */
        template <typename B>
        Serializer<B>::Serializer() : BaseFactory<B>() {
            baseObject = nullptr;
        }
        /** \fn void setObject(B* baseObject)
        *   \brief set to polymoprhic object to serialize.
        *   \param B* baseObject : the polymoprhic object to serialize.
        */
        template <typename B>
        void Serializer<B>::setObject(B* baseObject) {
            this->baseObject = baseObject;
        }
        /** \fn void serialize(std::string funcName, std::string funcArgs, Archive & ar)
        *   \brief call the member's function of the dynamic type of the polymoprhic object to serialize its member's attributes into the given archive.
        *   \param std::string funName : the name of the function which'll serialize the datas.
        *   \param std::string funcArgs :the signature id of the function which'll serialize the datas.
        *   \param Archive & ar : the archive where the data'll be serialized.
        */
        template <typename B>
        template <typename Archive>
        void Serializer<B>::serialize(std::string funcName, std::string funcArgs, Archive& ar) {
            BaseFactory<B>::callFunction(typeid(*baseObject).name(), funcName, funcArgs, baseObject, std::ref(ar));
        }
        /**\fn B* sallocate (std::string typeName)
        *  \brief std::string typeName : allocate an object of the given dynamic type name.
        *  \param std::string typeName : the name of the dynamic type.
        *  \return B* : a pointer to the allocated object.
        */
        template <typename B>
        B* Serializer<B>::sallocate(std::string typeName) {
            return BaseFactory<B>::create(typeName);
        }
        /**\fn std::string getTypeName()
        *  \brief get the name of the dynamic type of the polymorphic object.
        *  \return std::string : the dynamic type of the object.
        */
        /*template <typename B>
        std::string Serializer<B>::getTypeName() {
            return BaseFactory<B>::getTypeName(baseObject);
        }*/
        
        template <typename Base>
        Registered<Base>::Key::Key() : Serializer<Base>() {}
        /**\fn void register_object (Base* object)
        *  \brief register a polymoprhic object to serialize.
        *  \param Base* object : the polymoprhic object to serialize.
        */
        template <typename Base>
        void Registered<Base>::Key::register_object(Base* object) {
            Serializer<Base>::setObject(object);
        }
        template <typename Base>
        Base* Registered<Base>::Key::allocate_object(std::string typeName) { return Serializer<Base>::sallocate(typeName); }
        /**\fn std::string getTypeName ()
        *  \brief get the dynamic type of the registered object.
        *  \return std::string the dynamic type of the registered object.
        */
       
        /** \fn void serialize_object (std::string funcName, std::string funcArgs, Archive & ar)
        *   \brief serialize the registered object into the given archive.
        *   \param std::string funName : the name of the function which'll serialize the datas.
        *   \param std::string funcArgs :the signature id of the function which'll serialize the datas.
        *   \param Archive & ar : the archive where the data'll be serialized.
        */
        template <typename Base>
        template <typename Archive>
        void Registered<Base>::Key::serialize_object(std::string funcName, std::string funcArgs, Archive& ar) {
            Serializer<Base>::serialize(funcName, funcArgs, ar);
        }
        template <typename Base>
        Base* Registered<Base>::allocate(std::string typeName) {
            static KEYTYPE aKey;
            return aKey.allocate_object(typeName);
        }
    }
}