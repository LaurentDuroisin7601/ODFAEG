#ifndef ODFAEG_ARCHIVE
#define ODFAEG_ARCHIVE
#include <vector>
#include <map>
#include <iostream>
#include <typeinfo>
#include "factory.h"
#include <sstream>
#include <memory>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "resourceCache.h"
#include "export.hpp"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace core {
        /**
        * \file archive.h
        * \class has_typedef_key
        * \brief this struct is used by SFINAE to check if a class has the special typedef KEYTYPE which is used to serialize polymorphic objects.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        template <typename T>
        struct has_typedef_key {
            // Types "yes" and "no" are guaranteed to have different sizes,
            // specifically sizeof(yes) == 1 and sizeof(no) == 2.
            typedef char yes[1]; /**> yes type.*/
            typedef char no[2]; /** no type.*/
            /**\fn first case the compiler choose this function if the class C has the typedef KEYTYPE
            *  \param C::KEYTYPE* the key type.
            *  \return the yes type.
            */
            template <typename C>
            static yes& test(typename C::KEYTYPE*);
            /**\fn second case the compiler choose this function if the class C has no typedef KEYTYPE
            *  \param ... (we can use a var arg here we don't matter of the params.)
            *  \return the no type.
            */
            template <typename>
            static no& test(...);

            // If the "sizeof" of the result of calling test<T>(0) would be equal to sizeof(yes),
            // the first overload worked and T has a nested type named foobar.
            static const int value = sizeof(test<T>(0)) == sizeof(yes); /**> if the class has a typedef named KEYTYPE.*/
        };
        /**
        * \file archive.h
        * \class OTextArchive
        * \brief Write everything into the output archive's buffer in text format.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        class ODFAEG_CORE_API OTextArchive {
        public :
            /**\fn OTextArchive (std::ostream& buffer)
            *  \brief pass a c++ ouptut buffer to the archive.
            *  \param std::ostream& buffer : the output buffer.
            */
            OTextArchive(std::ostream& buffer) : buffer(buffer) {
                nbSerialized = 0;
            }
            /** \fn bool isInputArchive()
            *   \brief check if the archive is an input archive.
            *   \return false because it's an output archive.
            */
            bool isInputArchive() {
                return false;
            }
            /** \fn void clear()
            *   \brief clear the output stream buffer and the registered pointer adresses.
            */
            void clear() {
                buffer.clear();
                adresses.clear();
                nbSerialized = 0;
            }
            //Fundamentals.
            /**
            * \fn void operator(T& data, D...)
            * \brief write a fundamental type into the archive.
            * \param T& the data to write.
            * \param D... used for SFINAE.
            */
            template <typename T,
                      class... D,
                      class = typename std::enable_if<std::is_fundamental<T>::value>::type>
            void operator() (T& data, D...) {
                //////std::cout<<"write fundamental : "<<data<<std::endl;
                buffer<<data<<std::endl;
                nbSerialized++;
            }
            //Fundamentals.
            /**
            * \fn void operator(std::reference_wrapper<T> data, D...)
            * \brief write a fundamental type into the archive.
            * \param std::reference_wrapper<T> reference to the data to write.
            * \param D... used for SFINAE.
            */
            template <typename T,
                      class... D,
                      class = typename std::enable_if<std::is_fundamental<T>::value>::type>
            void operator() (std::reference_wrapper<T> ref, D...) {
                T& data = ref.get();
                //We need to get the type name (because object and first object member's variable have the same address) and the address of the data to write.
                std::ostringstream oss;
                oss<<typeid(data).name()<<"*"<<reinterpret_cast<unsigned long long int>(&data);
                //We check if the address of the data is already registered in the archive.
                std::map<std::string, long long int>::iterator it = adresses.find(oss.str());
                if (it != adresses.end()) {
                    //If the address of the data is already registered, we just write the address of the data to the buffer.
                    //////std::cout<<"write ref to fundamental type : "<<it->second<<std::endl;
                    buffer<<it->second<<std::endl;
                } else {
                    //If the address of the data is not registered, we register it's address and we write the data's value.
                    std::pair<std::string, long long int> newAddress (oss.str(), nbSerialized);
                    adresses.insert(newAddress);
                    buffer<<newAddress.second<<std::endl;
                    //////std::cout<<"write ref to fundamental type : "<<it->second<<std::endl;
                    buffer<<data<<std::endl;
                    //////std::cout<<"write ref to fundamental type : "<<data<<std::endl;
                    nbSerialized++;
                }
            }
            /**
            * \fn void operator(T* data, D...)
            * \brief write pointer to a fundamental type into the archive.
            * \param T* the pointer to write.
            * \param D... used for SFINAE.
            */
            template <typename T,
                  class... D,
                  class = typename std::enable_if<std::is_fundamental<T>::value>::type>
            void operator() (T* data, D...) {
                if (data != nullptr) {
                    std::ostringstream oss;
                    oss<<typeid(*data).name()<<"*"<<reinterpret_cast<unsigned long long int>(data);
                    std::map<std::string, long long int>::iterator it = adresses.find(oss.str());
                    if (it != adresses.end()) {
                        //////std::cout<<"id : "<<it->second<<std::endl;
                        buffer<<it->second<<std::endl;
                        //////std::cout<<"write pointer to a fundamental type id : "<<it->second<<std::endl;
                    } else {
                        std::pair<std::string, long long int> newAddress (oss.str(), nbSerialized);
                        adresses.insert(newAddress);
                        buffer<<newAddress.second<<std::endl;
                        //////std::cout<<"write pointer to a fundamental type id : "<<newAddress.second<<std::endl;
                        buffer<<(*data)<<std::endl;
                        //////std::cout<<"fundamental data : "<<(*data)<<std::endl;
                        nbSerialized++;
                    }
                } else {
                    long long int id = -1;
                    buffer<<id<<std::endl;
                }
            }
            /** \fn void operator() (E& data, D...)
            *   \brief write an enum value to the archive.
            *   \param E& data : the data to write.
            */
            template <typename E,
            class... D,
            class = typename std::enable_if<!std::is_fundamental<E>::value>::type,
            class = typename std::enable_if<std::is_enum<E>::value>::type>
            void operator() (E& data, D...) {
                buffer<<data<<std::endl;
                //////std::cout<<"write enum : "<<data<<std::endl;
                nbSerialized++;
            }
            /** \fn void operator() (E& data, D...)
            *   \brief write an enum value to the archive.
            *   \param std::reference_wrapper<E> ref : the reference to the data to write.
            */
            template <typename E,
            class... D,
            class = typename std::enable_if<!std::is_fundamental<E>::value>::type,
            class = typename std::enable_if<std::is_enum<E>::value>::type>
            void operator() (std::reference_wrapper<E> ref, D...) {
                E& data = ref.get();
                std::ostringstream oss;
                oss<<typeid(data).name()<<"*"<<reinterpret_cast<unsigned long long int>(&data);
                std::map<std::string, long long int>::iterator it = adresses.find(oss.str());
                if (it != adresses.end()) {
                    buffer<<it->second<<std::endl;
                    //////std::cout<<"write ref to enum : "<<it->second<<std::endl;
                } else {
                    std::pair<std::string, long long int> newAddress (oss.str(), nbSerialized);
                    adresses.insert(newAddress);
                    buffer<<newAddress.second<<std::endl;
                    //////std::cout<<"write ref to enum : "<<newAddress.second<<std::endl;
                    buffer<<data<<std::endl;
                    //////std::cout<<"write ref to enum : "<<data<<std::endl;
                    nbSerialized++;
                }
            }
            /** \fn void operator() (E& data, D...)
            *   \brief write an enum value to the archive.
            *   \param E* data : pointer to the data to write.
            */
            template <typename E,
            class... D,
            class = typename std::enable_if<!std::is_fundamental<E>::value>::type,
            class = typename std::enable_if<std::is_enum<E>::value>::type>
            void operator() (E* data, D...) {
                if (data != nullptr) {
                    std::ostringstream oss;
                    oss<<typeid(*data).name()<<"*"<<reinterpret_cast<unsigned long long int>(data);
                    std::map<std::string, long long int>::iterator it = adresses.find(oss.str());
                    if (it != adresses.end()) {
                        buffer<<it->second<<std::endl;
                        //////std::cout<<"write pointer to enum : "<<it->second<<std::endl;
                    } else {
                        std::pair<std::string, long long int> newAddress (oss.str(), nbSerialized);
                        adresses.insert(newAddress);
                        buffer<<newAddress.second<<std::endl;
                        //////std::cout<<"write pointer to enum : "<<newAddress.second<<std::endl;
                        buffer<<(*data)<<std::endl;
                         //////std::cout<<"data : "<<*data<<std::endl;
                        nbSerialized++;
                    }
                } else {
                    long long int id = -1;
                    buffer<<id<<std::endl;
                }
            }
            //std::string.
            /**
            *\fn void operator(T& data, D...)
            *\brief write an std::string into the archive.
            *\param T& data : the std::string to write.
            *\param D... : used fo SFINAE.
            */
            template <typename T,
                  class... D,
                  class = typename std::enable_if<!std::is_fundamental<T>::value>::type,
                  class = typename std::enable_if<std::is_same<T, std::string>::value || std::is_same<T, const std::string>::value>::type,
                  class = typename std::enable_if<!std::is_enum<T>::value>::type>
            void operator() (T& data, D...) {
                std::size_t str_size = data.length();
                buffer<<str_size<<std::endl;
                //////std::cout<<"write string : "<<str_size<<std::endl;
                const char* datas = data.c_str();
                nbSerialized++;
                for (unsigned int i = 0; i < str_size; i++) {
                    (*this)(datas[i]);
                }
            }
            /**
            *\fn void operator(T& data, D...)
            *\brief write an std::string into the archive.
            *\param std::reference_wrapper<T> : the reference to the std::string to write.
            *\param D... : used fo SFINAE.
            */
            template <typename T,
                  class... D,
                  class = typename std::enable_if<!std::is_fundamental<T>::value>::type,
                  class = typename std::enable_if<std::is_same<T, std::string>::value || std::is_same<T, const std::string>::value>::type,
                  class = typename std::enable_if<!std::is_enum<T>::value>::type>
            void operator() (std::reference_wrapper<T> ref, D...) {
                T& data = ref.get();
                std::ostringstream oss;
                oss<<typeid(data).name()<<"*"<<reinterpret_cast<unsigned long long int>(&data);
                std::map<std::string, long long int>::iterator it = adresses.find(oss.str());
                if (it != adresses.end()) {
                    buffer<<it->second<<std::endl;
                    //////std::cout<<"write ref to string : "<<it->second<<std::endl;
                } else {
                    std::pair<std::string, long long int> newAddress (oss.str(), nbSerialized);
                    adresses.insert(newAddress);
                    buffer<<newAddress.second<<std::endl;
                     //////std::cout<<"write ref to enum : "<<newAddress.second<<std::endl;
                    std::size_t str_size = data.length();
                    buffer<<str_size<<std::endl;
                     //////std::cout<<"write ref to enum : "<<str_size<<std::endl;
                    const char* datas = data.c_str();
                    nbSerialized++;
                    for (unsigned int i = 0; i < str_size; i++) {
                        (*this)(datas[i]);
                    }
                }
            }
            /**
            *\fn void operator(T* data, D...)
            *\brief The pointer to the std::string to write.
            *\param T* data : the pointer to the data to write.
            *\param D... : used for SFINAE.
            */
            template <typename T,
                  class... D,
                  class = typename std::enable_if<!std::is_fundamental<T>::value>::type,
                  class = typename std::enable_if<std::is_same<T, std::string>::value>::type,
                  class = typename std::enable_if<!std::is_enum<T>::value>::type>
            void operator() (T* data, D...) {
                if (data != nullptr) {
                    std::ostringstream oss;
                    oss<<typeid(*data).name()<<"*"<<reinterpret_cast<unsigned long long int>(data);
                    std::map<std::string, long long int>::iterator it = adresses.find(oss.str());
                    if (it != adresses.end()) {
                        buffer<<it->second<<std::endl;
                        //////std::cout<<"write pointer to string id : "<<it->second<<std::endl;
                    } else {
                        std::pair<std::string, long long int> newAddress (oss.str(), nbSerialized);
                        adresses.insert(newAddress);
                        buffer<<newAddress.second<<std::endl;
                        //////std::cout<<"write pointer to string id : "<<newAddress.second<<std::endl;
                        std::size_t str_size = data->length();
                        buffer<<str_size<<std::endl;
                        //////std::cout<<"str size : "<<str_size<<std::endl;
                        const char* datas = data->c_str();
                        nbSerialized++;
                        for (unsigned int i = 0; i < str_size; i++)
                            (*this)(datas[i]);
                    }
                } else {
                    long long int id = -1;
                    buffer<<id<<std::endl;
                }
            }
            //Static objects.
            /**
            *\fn void operator(O& data, D...)
            *\brief register a static object onto the archive.
            *\param O& the object to register.
            *\param D... : used for SFINAE.
            */
            template <class O,
                      class... D,
                      class = typename std::enable_if<!std::is_fundamental<O>::value>::type,
                      class = typename std::enable_if<!std::is_same<O, std::string>::value && !std::is_same<O, const std::string>::value && !std::is_pointer<O>::value>::type,
                      class = typename std::enable_if<!has_typedef_key<O>::value>::type,
                      class = typename std::enable_if<!std::is_enum<O>::value>::type>
            void operator() (O& object, D...) {
                //////std::cout<<"write static object : "<<std::endl;
                nbSerialized++;
                object.serialize(*this);
            }
             //Static objects.
            /**
            *\fn void operator(O& data, D...)
            *\brief register a static object onto the archive.
            *\param std::reference_wrapper<O> the reference to the object to register.
            *\param D... : used for SFINAE.
            */
            template <class O,
                      class... D,
                      class = typename std::enable_if<!std::is_fundamental<O>::value>::type,
                      class = typename std::enable_if<!std::is_same<O, std::string>::value && !std::is_same<O, const std::string>::value && !std::is_pointer<O>::value>::type,
                      class = typename std::enable_if<!has_typedef_key<O>::value>::type,
                      class = typename std::enable_if<!std::is_enum<O>::value>::type>
            void operator() (std::reference_wrapper<O> ref, D...) {
                O& object = ref.get();
                std::ostringstream oss;
                oss<<typeid(object).name()<<"*"<<reinterpret_cast<unsigned long long int>(&object);
                std::map<std::string, long long int>::iterator it = adresses.find(oss.str());
                if (it != adresses.end()) {
                    buffer<<it->second<<std::endl;
                    //////std::cout<<"write ref to static object : "<<it->second<<std::endl;
                } else {
                    std::pair<std::string, long long int> newAddress (oss.str(), nbSerialized);
                    adresses.insert(newAddress);
                    buffer<<newAddress.second<<std::endl;
                    //////std::cout<<"write ref to static object : "<<newAddress.second<<std::endl;
                    nbSerialized++;
                    object.serialize(*this);
                }
            }
            /**
            *\fn void operator(O* data, D...)
            *\brief register a static object onto the archive.
            *\param O* the pointer to the object to register.
            *\param D... : used for SFINAE.
            */
            template <class O,
                      class... D,
                      class = typename std::enable_if<!std::is_fundamental<O>::value>::type,
                      class = typename std::enable_if<!std::is_same<O, std::string>::value>::type,
                      class = typename std::enable_if<!has_typedef_key<O>::value>::type,
                      class = typename std::enable_if<!std::is_enum<O>::value>::type>
            void operator() (O* object, D...) {
                if (object != nullptr) {
                    std::ostringstream oss;
                    oss<<typeid(*object).name()<<"*"<<reinterpret_cast<unsigned long long int>(object);
                    std::map<std::string, long long int>::iterator it = adresses.find(oss.str());
                    if (it != adresses.end()) {
                        buffer<<it->second<<std::endl;
                        //////std::cout<<"write pointer static object : "<<it->second<<std::endl;
                    } else {
                        std::pair<std::string, long long int> newAddress (oss.str(), nbSerialized);
                        adresses.insert(newAddress);
                        buffer<<newAddress.second<<std::endl;
                        //////std::cout<<"write pointer static object : "<<newAddress.second<<std::endl;
                        nbSerialized++;
                        object->serialize(*this);
                    }
                } else {
                    //////std::cout<<"write null static object"<<std::endl;
                    long long int id = -1;
                    buffer<<id<<std::endl;
                }
            }
            //Dynamic objects.
            /**
            *\fn void operator(O& data, D...)
            *\brief register a dynamic object onto the archive.
            *\param O& the object to register.
            *\param D... : used for SFINAE.
            */
            template <class O,
                      class... D,
                      class = typename std::enable_if<!std::is_fundamental<O>::value>::type,
                      class = typename std::enable_if<!std::is_same<O, std::string>::value>::type,
                      class = typename std::enable_if<has_typedef_key<O>::value>::type,
                      class = typename std::enable_if<!std::is_enum<O>::value>::type,
                      class = typename std::enable_if<!sizeof...(D)>::type>
            void operator() (O& object, D...) {
                nbSerialized++;
                //////std::cout<<"write dynamic object"<<std::endl;
                //If the type at compile time is the same than the type at runtime.
                if (typeid(decltype(object)).name() == typeid(object).name()) {
                    //call the virtual template serialize function from the class.
                    object.vtserialize(*this);
                } else {
                    //the object is polymorphic so we need to register the object type at runtime and call the virtual template serialize function from the derived class.
                    object.key.register_object(&object);
                    object.key.serialize_object("serialize", "OTextArchive", *this);
                }
            }
            /**
            *\fn void operator(O& data, D...)
            *\brief register a dynamic object onto the archive.
            *\param std::reference_wrapper<O> the reference to the object to register.
            *\param D... : used for SFINAE.
            */
            template <class O,
                      class... D,
                      class = typename std::enable_if<!std::is_fundamental<O>::value>::type,
                      class = typename std::enable_if<!std::is_same<O, std::string>::value>::type,
                      class = typename std::enable_if<has_typedef_key<O>::value>::type,
                      class = typename std::enable_if<!std::is_enum<O>::value>::type,
                      class = typename std::enable_if<!sizeof...(D)>::type>
            void operator() (std::reference_wrapper<O> ref, D...) {
                O& object = ref.get();
                std::ostringstream oss;
                oss<<typeid(object).name()<<"*"<<reinterpret_cast<unsigned long long int>(&object);
                std::map<std::string, long long int>::iterator it = adresses.find(oss.str());
                if (it != adresses.end()) {
                    buffer<<it->second<<std::endl;
                    //////std::cout<<"write ref to dynamic object : "<<it->second<<std::endl;
                } else {
                    std::pair<std::string, long long int> newAddress (oss.str(), nbSerialized);
                    adresses.insert(newAddress);
                    buffer<<newAddress.second<<std::endl;
                    //////std::cout<<"write ref to dynamic object : "<<newAddress.second<<std::endl;
                    nbSerialized++;
                    if (typeid(decltype(object)).name() == typeid(object).name()) {
                        object.vtserialize(*this);
                    } else {
                        object.key.register_object(&object);
                        object.key.serialize_object("serialize", "OTextArchive", *this);
                    }
                }
            }
            /**
            *\fn void operator(O& data, D...)
            *\brief register pointer to a dynamic object onto the archive.
            *\param O& the object to register.
            *\param D... : used for SFINAE.
            */
            template <class O,
                      class... D,
                      class = typename std::enable_if<!std::is_fundamental<O>::value>::type,
                      class = typename std::enable_if<!std::is_same<O, std::string>::value>::type,
                      class = typename std::enable_if<has_typedef_key<O>::value>::type,
                      class = typename std::enable_if<!std::is_enum<O>::value>::type,
                      class = typename std::enable_if<!sizeof...(D)>::type>
            void operator() (O* object, D...) {
                if (object != nullptr) {
                    std::ostringstream oss;
                    oss<<typeid(*object).name()<<"*"<<reinterpret_cast<unsigned long long int>(object);
                    std::map<std::string, long long int>::iterator it = adresses.find(oss.str());
                    if (it != adresses.end()) {
                        buffer<<it->second<<std::endl;
                        //////std::cout<<"write dynamic object : "<<it->second<<std::endl;
                    } else {
                        std::pair<std::string, long long int> newAddress (oss.str(), nbSerialized);
                        adresses.insert(newAddress);
                        buffer<<newAddress.second<<std::endl;
                        //////std::cout<<"write dynamic object : "<<newAddress.second<<std::endl;
                        std::string typeName = "BaseType";
                        if (typeid(decltype(*object)).name() == typeid(*object).name()) {
                            buffer<<typeName<<std::endl;
                            //////std::cout<<"type name : "<<typeName<<std::endl;
                            nbSerialized++;
                            object->vtserialize(*this);
                        } else {
                            object->key.register_object(object);
                            typeName = object->key.getTypeName();
                            buffer<<typeName<<std::endl;
                            //////std::cout<<"type name : "<<typeName<<std::endl;
                            nbSerialized++;
                            object->key.serialize_object("serialize", "OTextArchive", *this);
                        }
                    }
                } else {
                    //////std::cout<<"serialize null dynamic object"<<std::endl;
                    long long int id = -1;
                    buffer<<id<<std::endl;
                }
            }
            //std::vectors.
            /**
            *\fn void operator(std::vector<O>&, D...)
            *\brief register a list of objects onto the archive.
            *\param std::vector<O>& the list of objects to register.
            *\param D... : used for SFINAE.
            */
            template <class O>
            void operator() (std::vector<O>& data) {
                std::size_t size = data.size();
                buffer<<size<<std::endl;
                for (unsigned int i = 0; i < data.size(); i++)
                     (*this)(data[i]);
            }
            /**
            *\fn void operator(std::unique_ptr<T>&, D...)
            *\brief register a std::unique_ptr onto the archive.
            *\param std::unique_ptr<T>& the unique pointer.
            *\param D... : used for SFINAE.
            */
            template <class T>
            void operator() (std::unique_ptr<T>& ptr) {
                (*this)(ptr.get());
            }
            /**
            *\fn void operator(std::pair<T1, T2>&, D...)
            *\brief register an std::pair onto the archive.
            *\param std::pair<T1, T2>& the std::pair.
            *\param D... : used for SFINAE.
            */
            template <class T1, class T2>
            void operator() (std::pair<T1, T2>& pair) {
                (*this)(pair.first);
                (*this)(pair.second);
            }
            /**
            *\fn void operator(std::map<T1, T2>&, D...)
            *\brief register an std::map onto the archive.
            *\param std::map<T1, T2>& the std::map.
            *\param D... : used for SFINAE.
            */
            template <class T1, class T2>
            void operator() (std::map<T1, T2>&  map) {
                std::size_t size = map.size();
                buffer<<size<<std::endl;
                typename std::map<T1, T2>::iterator it;
                for (it = map.begin(); it != map.end(); it++) {
                    (*this)(*it);
                }
            }
        private :
            std::ostream& buffer; /**< the output buffer containing the datas.*/
            std::map<std::string, long long int> adresses; /**< an std::map used to store the adresses and the id of the serialized pointers.*/
            unsigned long long int nbSerialized; /** <the number data which are serialized into the archive*/
        };
        /**
        * \file archive.h
        * \class ITextArchive
        * \brief Read everything from the input archive's buffer.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        class ODFAEG_CORE_API ITextArchive {
        public :
            /**
            *\fn ITextArchive(std::istream& buffer)
            *\brief pass the input stream to the input test archive.
            *\param std::istream& buffer : the input buffer where to read the datas.
            */
            ITextArchive (std::istream& buffer) : buffer(buffer) {
                nbDeserialized = 0;
            }
            /**
            * \fn bool isAtEndOfInputStream()
            * \brief check if all the data has been extracted from the archive.
            * \return if all the data has been extracted from the archive.
            */
            bool isAtEndOfInputStream() {
                return buffer.eof();
            }
            /**
            *\fn bool isInputArchive()
            * \brief check if the archive is an input archive.
            * \return true because this is an input archive.
            */
            bool isInputArchive() {
                return true;
            }
            /**
            * \fn void clear()
            * \brief clear the input stream buffer, and registered addresses.
            */
            void clear() {
                buffer.clear();
                adresses.clear();
                nbDeserialized = 0;
            }
            //Fundamentals.
            /**
            * \fn void operator(T& data, D...)
            * \brief read a fundamental type from the archive.
            * \param T& the data to read.
            * \param D... used for SFINAE.
            */
            template <typename T,
                  class... D,
                  class = typename std::enable_if<std::is_fundamental<T>::value>::type>
            void operator() (T& data, D...) {

                nbDeserialized++;
                buffer>>data;
                //////std::cout<<"read fundamental : "<<data<<std::endl;
                char space;
                buffer.get(space);
            }
            //Fundamentals.
            /**
            * \fn void operator(T& data, D...)
            * \brief read a fundamental type from the archive.
            * \param std::reference_wrapper<T> reference to the data to read.
            * \param D... used for SFINAE.
            */
            template <typename T,
                  class... D,
                  class = typename std::enable_if<std::is_fundamental<T>::value>::type>
            void operator() (std::reference_wrapper<T> ref, D...) {
                T& data = ref.get();
                long long int id;
                buffer>>id;
                //////std::cout<<"read reference to fundamental, id : "<<id<<std::endl;
                char space;
                buffer.get(space);
                std::map<long long int, std::string>::iterator it = adresses.find(id);
                if (it != adresses.end()) {
                    std::istringstream iss(it->second);
                    std::vector<std::string> parts = split(iss.str(), "*");
                    data = *reinterpret_cast<T*> (conversionStringULong(parts[1]));
                } else {
                    std::ostringstream oss;
                    oss<<typeid(data).name()<<"*"<<reinterpret_cast<unsigned long long int>(&data);
                    std::pair<long long int, std::string> newAddress (id, oss.str());
                    adresses.insert(newAddress);
                    nbDeserialized++;
                    buffer>>data;
                    //////std::cout<<"data : "<<data<<std::endl;
                    char space;
                    buffer.get(space);
                }
            }
            /**
            * \fn void operator(T& data, D...)
            * \brief read a char from the archive. (we need to read unformatted input here to also read special chars like \n, spaces, etc...)
            * \param T& the data to read.
            * \param D... used for SFINAE.
            */

            void operator() (char& data) {
                nbDeserialized++;
                //////std::cout<<"read char : "<<data<<std::endl;
                buffer.get(data);
                //////std::cout<<"data"<<std::endl;
                char space;
                buffer.get(space);
            }
            /**
            * \fn void operator(T& data, D...)
            * \brief read a char from the archive. (we need to read unformatted input here to also read special chars like \n, spaces, etc...)
            * \param std::reference_wrapper<char> reference to the data to read.
            * \param D... used for SFINAE.
            */

            void operator() (std::reference_wrapper<char> ref) {
                char& data = ref.get();
                long long int id;
                buffer>>id;
                //////std::cout<<"read reference to char : "<<id<<std::endl;
                char space;
                buffer.get(space);
                std::map<long long int, std::string>::iterator it = adresses.find(id);
                if (it != adresses.end()) {
                    std::istringstream iss(it->second);
                    std::vector<std::string> parts = split(iss.str(), "*");
                    data = *reinterpret_cast<char*> (conversionStringULong(parts[1]));
                } else {
                    std::ostringstream oss;
                    oss<<typeid(data).name()<<"*"<<reinterpret_cast<unsigned long long int>(&data);
                    std::pair<long long int, std::string> newAddress (id, oss.str());
                    adresses.insert(newAddress);
                    nbDeserialized++;
                    buffer.get(data);
                    //////std::cout<<"data : "<<data<<std::endl;
                    char space;
                    buffer.get(space);
                }
            }
            /**
            * \fn void operator(T& data, D...)
            * \brief read an unsigned char from the archive. (we need to read unformatted input here to also read special chars like \n, spaces, etc...)
            * \param T& the data to read.
            * \param D... used for SFINAE.
            */

            void operator() (unsigned char& data) {
                nbDeserialized++;
                buffer.get((char&) data);
                //////std::cout<<"read unsigned char : "<<data<<std::endl;
                char space;
                buffer.get(space);
            }
            /**
            * \fn void operator(T& data, D...)
            * \brief read an unsigned char from the archive. (we need to read unformatted input here to also read special chars like \n, spaces, etc...)
            * \param std::reference_wrapper<unsigned char> reference the data to read.
            * \param D... used for SFINAE.
            */

            void operator() (std::reference_wrapper<unsigned char> ref) {
                unsigned char& data = ref.get();
                long long int id;
                buffer>>id;
                //////std::cout<<"read reference to unsigned char : "<<id<<std::endl;
                char space;
                buffer.get(space);
                std::map<long long int, std::string>::iterator it = adresses.find(id);
                if (it != adresses.end()) {
                    std::istringstream iss(it->second);
                    std::vector<std::string> parts = split(iss.str(), "*");
                    data = *reinterpret_cast<unsigned char*> (conversionStringULong(parts[1]));
                } else {
                    std::ostringstream oss;
                    oss<<typeid(data).name()<<"*"<<reinterpret_cast<unsigned long long int>(&data);
                    std::pair<long long int, std::string> newAddress (id, oss.str());
                    adresses.insert(newAddress);
                    nbDeserialized++;
                    buffer.get((char&) data);
                    //////std::cout<<"data : "<<data<<std::endl;
                    char space;
                    buffer.get(space);
                }
            }
            /**
            * \fn void operator(T& data, D...)
            * \brief read a pointer to a fundamental type from the archive.
            * \param T& the data to read.
            * \param D... used for SFINAE.
            */
            template <typename T,
                  class... D,
                  class = typename std::enable_if<std::is_fundamental<T>::value>::type>
            void operator() (T*& data, D...) {
                long long int id;
                buffer>>id;
                char space;
                buffer.get(space);
                //////std::cout<<"read pointer to fundamental id : "<<id<<std::endl;
                if (id != -1) {
                    std::map<long long int, std::string>::iterator it = adresses.find(id);
                    if (it != adresses.end()) {
                        std::istringstream iss(it->second);
                        std::vector<std::string> parts = split(iss.str(), "*");
                        data = reinterpret_cast<T*> (conversionStringULong(parts[1]));
                    } else {
                        data = new T();
                        std::ostringstream oss;
                        oss<<typeid(*data).name()<<"*"<<reinterpret_cast<unsigned long long int>(data);
                        std::pair<long long int, std::string> newAddress (id, oss.str());
                        adresses.insert(newAddress);
                        nbDeserialized++;
                        buffer>>(*data);
                        //////std::cout<<"fundamental data : "<<(*data)<<std::endl;
                        char space;
                        buffer.get(space);
                    }
                } else {
                    data = nullptr;
                }
            }
             /**
            * \fn void operator(T& data, D...)
            * \brief read a pointer to a fundamental type from the archive.
            * \param T& the data to read.
            * \param D... used for SFINAE.
            */
            template <typename T,
                  class... D,
                  class = typename std::enable_if<std::is_fundamental<T>::value>::type>
            void operator() (char*& data, D...) {
                long long int id;
                buffer>>id;
                char space;
                buffer.get(space);
                //////std::cout<<"read pointer to char : "<<id<<std::endl;
                if (id != -1) {
                    std::map<long long int, std::string>::iterator it = adresses.find(id);
                    if (it != adresses.end()) {
                        std::istringstream iss(it->second);
                        std::vector<std::string> parts = split(iss.str(), "*");
                        data = reinterpret_cast<T*> (conversionStringULong(parts[1]));
                    } else {
                        data = new char();
                        std::ostringstream oss;
                        oss<<typeid(*data).name()<<"*"<<reinterpret_cast<unsigned long long int>(data);
                        std::pair<long long int, std::string> newAddress (id, oss.str());
                        adresses.insert(newAddress);
                        nbDeserialized++;
                        buffer>>(*data);
                        char space;
                        buffer.get(space);
                        //////std::cout<<"char data : "<<(*data)<<std::endl;
                    }
                } else {
                    data = nullptr;
                }
            }
             /**
            * \fn void operator(T& data, D...)
            * \brief read a pointer to a fundamental type from the archive.
            * \param T& the data to read.
            * \param D... used for SFINAE.
            */
            template <typename T,
                  class... D,
                  class = typename std::enable_if<std::is_fundamental<T>::value>::type>
            void operator() (unsigned char*& data, D...) {
                long long int id;
                buffer>>id;
                char space;
                buffer.get(space);
                //////std::cout<<"read pointer to unsigned char id : "<<id<<std::endl;
                if (id != -1) {
                    std::map<long long int, std::string>::iterator it = adresses.find(id);
                    if (it != adresses.end()) {
                        std::istringstream iss(it->second);
                        std::vector<std::string> parts = split(iss.str(), "*");
                        data = reinterpret_cast<T*> (conversionStringULong(parts[1]));
                    } else {
                        data = new unsigned char();
                        std::ostringstream oss;
                        oss<<typeid(*data).name()<<"*"<<reinterpret_cast<unsigned long long int>(data);
                        std::pair<long long int, std::string> newAddress (id, oss.str());
                        adresses.insert(newAddress);
                        nbDeserialized++;
                        buffer>>((char&) *data);
                        char space;
                        buffer.get(space);
                        //////std::cout<<"unsigned char data"<<(*data)<<std::endl;
                    }
                } else {
                    data = nullptr;
                }
            }
            /**
            * \fn void operator()(E& data, D...)
            * \brief read an enum value from the archive.
            * \param E& the data the read.
            */
            template <typename E,
                    class... D,
                    class = typename std::enable_if<!std::is_fundamental<E>::value>::type,
                    class = typename std::enable_if<std::is_enum<E>::value>::type>
            void operator()(E& data, D...) {
                int eVal;
                buffer>>eVal;
                data = static_cast<E>(eVal);
                //////std::cout<<"read enum : "<<data<<std::endl;
                char space;
                buffer.get(space);
                nbDeserialized++;
            }
            /**
            * \fn void operator()(E& data, D...)
            * \brief read an enum value from the archive.
            * \param std::reference_wrapper<E> the reference to the data the read.
            */
            template <typename E,
                    class... D,
                    class = typename std::enable_if<!std::is_fundamental<E>::value>::type,
                    class = typename std::enable_if<std::is_enum<E>::value>::type>
            void operator()(std::reference_wrapper<E> ref, D...) {
                E& data = ref.get();
                long long int id;
                buffer>>id;
                //////std::cout<<"read reference to enum id : "<<id<<std::endl;
                char space;
                buffer.get(space);
                std::map<long long int, std::string>::iterator it = adresses.find(id);
                if (it != adresses.end()) {
                    std::istringstream iss(it->second);
                    std::vector<std::string> parts = split(iss.str(), "*");
                    data = *reinterpret_cast<E*> (conversionStringULong(parts[1]));
                } else {
                    int eVal;
                    buffer>>eVal;
                    //////std::cout<<"val : "<<eVal<<std::endl;
                    data = static_cast<E>(eVal);
                    char space;
                    buffer.get(space);
                    std::ostringstream oss;
                    oss<<typeid(data).name()<<"*"<<reinterpret_cast<unsigned long long int>(data);
                    std::pair<long long int, std::string> newAddress (id, oss.str());
                    adresses.insert(newAddress);
                    nbDeserialized++;
                }
            }
            template <typename E,
                    class... D,
                    class = typename std::enable_if<!std::is_fundamental<E>::value>::type,
                    class = typename std::enable_if<std::is_enum<E>::value>::type>
            void operator() (E*& data, D...) {
                long long int id;
                buffer>>id;
                //////std::cout<<"read pointer to enum id : "<<id<<std::endl;
                char space;
                buffer.get(space);
                if (id != -1) {
                    std::map<long long int, std::string>::iterator it = adresses.find(id);
                    if (it != adresses.end()) {
                        std::istringstream iss(it->second);
                        std::vector<std::string> parts = split(iss.str(), "*");
                        data = reinterpret_cast<E*> (conversionStringULong(parts[1]));
                    } else {
                        data = new E();
                        int eVal;
                        buffer>>eVal;
                        //////std::cout<<"e val : "<<eVal<<std::endl;
                        *data = static_cast<E>(eVal);
                        char space;
                        buffer.get(space);
                        std::ostringstream oss;
                        oss<<typeid(*data).name()<<"*"<<reinterpret_cast<unsigned long long int>(data);
                        std::pair<long long int, std::string> newAddress (id, oss.str());
                        adresses.insert(newAddress);
                        nbDeserialized++;
                    }
                } else {
                    data = nullptr;
                }
            }
            //std::string.
            /**
            * \fn void operator(T& data, D...)
            * \brief read an std::string from the archive.
            * \param T& the data to read.
            * \param D... used for SFINAE.
            */
            template <typename T,
                  class... D,
                  class = typename std::enable_if<!std::is_fundamental<T>::value>::type,
                  class = typename std::enable_if<std::is_same<T, std::string>::value>::type,
                  class = typename std::enable_if<!std::is_enum<T>::value>::type>
            void operator() (T& data, D...) {
                /*long long int id;
                buffer>>id;
                char space;
                buffer.get(space);
                //////std::cout<<"id : "<<id<<std::endl;
                std::map<long long int, std::string>::iterator it = adresses.find(id);
                if (it != adresses.end()) {
                    std::istringstream iss(it->second);
                    std::vector<std::string> parts = split(iss.str(), "*");
                    data = *reinterpret_cast<T*> (conversionStringULong(parts[1]));
                } else {*/
                    std::size_t str_size;
                    buffer>>str_size;
                    //////std::cout<<"read string, size : "<<str_size<<std::endl;
                    if (str_size > 0) {
                        char space;
                        buffer.get(space);
                        //////std::cout<<"str size : "<<str_size<<std::endl;
                        char* datas = new char[str_size];
                        nbDeserialized++;
                        for (unsigned int i = 0; i < str_size; i++) {
                            (*this)(datas[i]);
                        }
                        data = std::string(datas, str_size);
                    }
                    /*std::ostringstream oss;
                    oss<<typeid(data).name()<<"*"<<reinterpret_cast<unsigned long long int>(&data);
                    std::pair<long long int, std::string> newAddress (id, oss.str());
                    adresses.insert(newAddress);*/
                //}
            }
             /**
            * \fn void operator(T& data, D...)
            * \brief read an std::string from the archive.
            * \param T& the data to read.
            * \param D... used for SFINAE.
            */
            template <typename T,
                  class... D,
                  class = typename std::enable_if<!std::is_fundamental<T>::value>::type,
                  class = typename std::enable_if<std::is_same<T, std::string>::value>::type,
                  class = typename std::enable_if<!std::is_enum<T>::value>::type>
            void operator() (std::reference_wrapper<T> ref, D...) {
                T& data = ref.get();
                long long int id;
                buffer>>id;
                char space;
                buffer.get(space);
                //////std::cout<<"read reference to std::string id : "<<id<<std::endl;
                std::map<long long int, std::string>::iterator it = adresses.find(id);
                if (it != adresses.end()) {
                    std::istringstream iss(it->second);
                    std::vector<std::string> parts = split(iss.str(), "*");
                    data = *reinterpret_cast<T*> (conversionStringULong(parts[1]));
                } else {
                    std::size_t str_size;
                    buffer>>str_size;
                    //////std::cout<<"str size : "<<str_size<<std::endl;
                    if (str_size > 0) {
                        char space;
                        buffer.get(space);
                        //////std::cout<<"str size : "<<str_size<<std::endl;
                        char* datas = new char[str_size];
                        nbDeserialized++;
                        for (unsigned int i = 0; i < str_size; i++) {
                            (*this)(datas[i]);
                        }
                        data = std::string(datas, str_size);
                    }
                    std::ostringstream oss;
                    oss<<typeid(data).name()<<"*"<<reinterpret_cast<unsigned long long int>(&data);
                    std::pair<long long int, std::string> newAddress (id, oss.str());
                    adresses.insert(newAddress);
                }
            }
            /**
            * \fn void operator(T& data, D...)
            * \brief read a pointer to an std::string from the archive.
            * \param T* the data to read.
            * \param D... used for SFINAE.
            */
            template <typename T,
                  class... D,
                  class = typename std::enable_if<!std::is_fundamental<T>::value>::type,
                  class = typename std::enable_if<std::is_same<T, std::string>::value>::type,
                  class = typename std::enable_if<!std::is_enum<T>::value>::type>
            void operator() (T*& data, D...) {
                long long int id;
                buffer>>id;
                char space;
                buffer.get(space);
                //////std::cout<<"read pointer to std::string id : "<<id<<std::endl;
                if (id != -1) {
                    std::map<long long int, std::string>::iterator it = adresses.find(id);
                    if (it != adresses.end()) {
                        std::istringstream iss(it->second);
                        std::vector<std::string> parts = split(iss.str(), "*");
                        data = reinterpret_cast<T*> (conversionStringULong(parts[1]));
                    } else {
                        std::size_t str_size;
                        buffer>>str_size;
                        //////std::cout<<"str size : "<<str_size<<std::endl;
                        if (str_size > 0) {
                            char space;
                            buffer.get(space);
                            //////std::cout<<"str size : "<<str_size<<std::endl;
                            char* datas = new char [str_size];
                            nbDeserialized++;
                            for (unsigned int i = 0; i < str_size; i++)
                                (*this)(datas[i]);
                            data = new std::string(datas, str_size);
                            std::ostringstream oss;
                            oss<<typeid(*data).name()<<"*"<<reinterpret_cast<unsigned long long int>(data);
                            std::pair<long long int, std::string> newAddress (id, oss.str());
                            adresses.insert(newAddress);
                        }
                    }
                } else {
                    data = nullptr;
                }
            }
            //Static objects.
            /**
            * \fn void operator(O& data, D...)
            * \brief read a static object from the archive.
            * \param O& the data to read.
            * \param D... used for SFINAE.
            */
            template <class O,
                      class... D,
                      class = typename std::enable_if<!std::is_fundamental<O>::value>::type,
                      class = typename std::enable_if<!std::is_same<O, std::string>::value && !std::is_pointer<O>::value>::type,
                      class = typename std::enable_if<!has_typedef_key<O>::value>::type,
                      class = typename std::enable_if<!std::is_enum<O>::value>::type>
            void operator() (O& object, D...) {
                /*long long int id;
                buffer>>id;
                char space;
                buffer.get(space);
                //////std::cout<<"id : "<<id<<std::endl;
                std::map<long long int, std::string>::iterator it = adresses.find(id);
                if (it != adresses.end()) {
                    std::istringstream iss(it->second);
                    std::vector<std::string> parts = split(iss.str(), "*");
                    object = *reinterpret_cast<O*> (conversionStringULong(parts[1]));
                } else {
                    std::ostringstream oss;
                    oss<<typeid(object).name()<<"*"<<reinterpret_cast<unsigned long long int>(&object);
                    std::pair<long long int, std::string> newAddress (id, oss.str());
                    adresses.insert(newAddress);*/
                    //////std::cout<<"read static object"<<std::endl;
                    nbDeserialized++;
                    object.serialize(*this);
                //}
            }
            /**
            * \fn void operator(O& data, D...)
            * \brief read a static object from the archive.
            * \param std::reference_wrapper<O> the reference to the data to read.
            * \param D... used for SFINAE.
            */
            template <class O,
                      class... D,
                      class = typename std::enable_if<!std::is_fundamental<O>::value>::type,
                      class = typename std::enable_if<!std::is_same<O, std::string>::value && !std::is_pointer<O>::value>::type,
                      class = typename std::enable_if<!has_typedef_key<O>::value>::type,
                      class = typename std::enable_if<!std::is_enum<O>::value>::type>
            void operator() (std::reference_wrapper<O> ref, D...) {
                O& object = ref.get();
                long long int id;
                buffer>>id;
                char space;
                buffer.get(space);
                //////std::cout<<"read reference to a static object id : "<<id<<std::endl;
                std::map<long long int, std::string>::iterator it = adresses.find(id);
                if (it != adresses.end()) {
                    std::istringstream iss(it->second);
                    std::vector<std::string> parts = split(iss.str(), "*");
                    object = *reinterpret_cast<O*> (conversionStringULong(parts[1]));
                } else {
                    std::ostringstream oss;
                    oss<<typeid(object).name()<<"*"<<reinterpret_cast<unsigned long long int>(&object);
                    std::pair<long long int, std::string> newAddress (id, oss.str());
                    adresses.insert(newAddress);
                    nbDeserialized++;
                    object.serialize(*this);
                }
            }
            /**
            * \fn void operator(O& data, D...)
            * \brief read a pointer to a static object from the archive.
            * \param O* the data to read.
            * \param D... used for SFINAE.
            */
            template <class O,
                      class... D,
                      class = typename std::enable_if<!std::is_fundamental<O>::value>::type,
                      class = typename std::enable_if<!std::is_same<O, std::string>::value>::type,
                      class = typename std::enable_if<!has_typedef_key<O>::value>::type,
                      class = typename std::enable_if<!std::is_enum<O>::value>::type>
            void operator() (O*& object, D...) {
                long long int id;
                buffer>>id;
                //////std::cout<<"read pointer to static object id : "<<id<<std::endl;
                char space;
                buffer.get(space);
                if (id != -1) {
                    std::map<long long int, std::string>::iterator it = adresses.find(id);
                    if (it != adresses.end()) {
                        std::istringstream iss(it->second);
                        std::vector<std::string> parts = split(iss.str(), "*");
                        object = reinterpret_cast<O*> (conversionStringULong(parts[1]));
                    } else {
                        object = new O();
                        std::ostringstream oss;
                        oss<<typeid(*object).name()<<"*"<<reinterpret_cast<unsigned long long int>(object);
                        std::pair<long long int, std::string> newAddress (id, oss.str());
                        adresses.insert(newAddress);
                        nbDeserialized++;
                        object->serialize(*this);
                    }
                } else {
                    object = nullptr;
                }
            }
            //Dynamic objects.
            /**
            * \fn void operator(O* data, D...)
            * \brief read a dynamic object from the archive.
            * \param O& the data to read.
            * \param D... used for SFINAE.
            */
            template <class O,
                      class... D,
                      class = typename std::enable_if<!std::is_fundamental<O>::value>::type,
                      class = typename std::enable_if<!std::is_same<O, std::string>::value>::type,
                      class = typename std::enable_if<has_typedef_key<O>::value>::type,
                      class = typename std::enable_if<!std::is_enum<O>::value>::type,
                      class = typename std::enable_if<!sizeof...(D)>::type>
            void operator() (O& object, D...) {
                /*long long int id;
                buffer>>id;
                char space;
                buffer.get(space);
                //////std::cout<<"id : "<<id<<std::endl;
                std::map<long long int, std::string>::iterator it = adresses.find(id);
                if (it != adresses.end()) {
                    std::istringstream iss(it->second);
                    std::vector<std::string> parts = split(iss.str(), "*");
                    object = *reinterpret_cast<O*> (conversionStringULong(parts[1]));
                } else {
                    std::ostringstream oss;
                    oss<<typeid(object).name()<<"*"<<reinterpret_cast<unsigned long long int>(&object);
                    std::pair<long long int, std::string> newAddress (id, oss.str());
                    adresses.insert(newAddress);*/
                    //////std::cout<<"read dynamic object : "<<std::endl;
                    nbDeserialized++;
                    if (typeid(decltype(object)) == typeid(object)) {
                        object.vtserialize(*this);
                    } else {
                        object.key.register_object(&object);
                        object.key.serialize_object("serialize", "ITextArchive", *this);
                    }
                //}
            }
            /**
            * \fn void operator(O* data, D...)
            * \brief read a dynamic object from the archive.
            * \param std::reference_wrapper<O> the reference to the data to read.
            * \param D... used for SFINAE.
            */
            template <class O,
                      class... D,
                      class = typename std::enable_if<!std::is_fundamental<O>::value>::type,
                      class = typename std::enable_if<!std::is_same<O, std::string>::value>::type,
                      class = typename std::enable_if<has_typedef_key<O>::value>::type,
                      class = typename std::enable_if<!std::is_enum<O>::value>::type,
                      class = typename std::enable_if<!sizeof...(D)>::type>
            void operator() (std::reference_wrapper<O> ref, D...) {
                O& object = ref.get();
                long long int id;
                buffer>>id;
                //////std::cout<<"read reference to dynamic object id : "<<id<<std::endl;
                char space;
                buffer.get(space);
                std::map<long long int, std::string>::iterator it = adresses.find(id);
                if (it != adresses.end()) {
                    std::istringstream iss(it->second);
                    std::vector<std::string> parts = split(iss.str(), "*");
                    object = *reinterpret_cast<O*> (conversionStringULong(parts[1]));
                } else {
                    std::ostringstream oss;
                    oss<<typeid(object).name()<<"*"<<reinterpret_cast<unsigned long long int>(&object);
                    std::pair<long long int, std::string> newAddress (id, oss.str());
                    adresses.insert(newAddress);
                    nbDeserialized++;
                    if (typeid(decltype(object)) == typeid(object)) {
                        object.vtserialize(*this);
                    } else {
                        object.key.register_object(&object);
                        object.key.serialize_object("serialize", "ITextArchive", *this);
                    }
                }
            }
            /**
            * \fn void operator(O* data, D...)
            * \brief read a pointer to a non abstract dynamic object from the archive.
            * \param O* the data to read.
            * \param D... used for SFINAE.
            */
            template <class O,
                      class... D,
                      class = typename std::enable_if<!std::is_fundamental<O>::value>::type,
                      class = typename std::enable_if<!std::is_same<O, std::string>::value>::type,
                      class = typename std::enable_if<!std::is_enum<O>::value>::type,
                      class = typename std::enable_if<has_typedef_key<O>::value && !std::is_abstract<O>::value>::type,
                      class = typename std::enable_if<!sizeof...(D)>::type>
            void operator() (O*& object, D...) {
                long long int id;
                buffer>>id;
                char space;
                buffer.get(space);
                //////std::cout<<"read pointer to a dynamic object id : "<<id<<std::endl;
                if (id != -1) {
                    std::map<long long int, std::string>::iterator it = adresses.find(id);
                    if (it != adresses.end()) {
                        std::istringstream iss(it->second);
                        std::vector<std::string> parts = split(iss.str(), "*");
                        object = reinterpret_cast<O*> (conversionStringULong(parts[1]));
                    } else {
                        std::string typeName;
                        getline(buffer, typeName);
                        if (typeName == "BaseType") {
                            object = new O();
                            std::ostringstream oss;
                            oss<<typeid(*object).name()<<"*"<<reinterpret_cast<unsigned long long int>(object);
                            std::pair<long long int, std::string> newAddress (id, oss.str());
                            adresses.insert(newAddress);
                            nbDeserialized++;
                            object->vtserialize(*this);
                        } else {
                            object = static_cast<O*>(O::allocate(typeName));
                            std::ostringstream oss;
                            oss<<typeid(*object).name()<<"*"<<reinterpret_cast<unsigned long long int>(object);
                            std::pair<long long int, std::string> newAddress (id, oss.str());
                            adresses.insert(newAddress);
                            nbDeserialized++;
                            object->key.register_object(object);
                            object->key.serialize_object("serialize", "ITextArchive", *this);
                        }
                    }
                } else {
                    object = nullptr;
                }
            }
            /**
            * \fn void operator(O* data, D...)
            * \brief read a pointer to an abstract dynamic object from the archive.
            * \param O* the data to read.
            * \param D... used for SFINAE.
            */
            template <class O,
                      class... D,
                      class = typename std::enable_if<!std::is_fundamental<O>::value>::type,
                      class = typename std::enable_if<!std::is_same<O, std::string>::value>::type,
                      class = typename std::enable_if<!std::is_enum<O>::value>::type,
                      class = typename std::enable_if<has_typedef_key<O>::value>::type,
                      class = typename std::enable_if<std::is_abstract<O>::value>::type,
                      class = typename std::enable_if<!sizeof...(D)>::type>
            void operator() (O*& object, D...) {
                long long int id;
                buffer>>id;
                char space;
                buffer.get(space);
                //////std::cout<<"read pointer to an abstract dynamic object id : "<<id<<std::endl;
                if (id != -1) {
                    std::map<long long int, std::string>::iterator it = adresses.find(id);
                    if (it != adresses.end()) {
                        std::istringstream iss(it->second);
                        std::vector<std::string> parts = split(iss.str(), "*");
                        object = reinterpret_cast<O*> (conversionStringULong(parts[1]));
                    } else {
                        std::string typeName;
                        getline(buffer, typeName);
                        object = static_cast<O*>(O::allocate(typeName));
                        std::ostringstream oss;
                        oss<<typeid(*object).name()<<"*"<<reinterpret_cast<unsigned long long int>(object);
                        std::pair<long long int, std::string> newAddress (id, oss.str());
                        adresses.insert(newAddress);
                        nbDeserialized++;
                        object->key.register_object(object);
                        object->key.serialize_object("serialize", "ITextArchive", *this);
                    }
                } else {
                    object = nullptr;
                }
            }
            /**
            * \fn void operator(O* data, D...)
            * \brief read a list of objects from the archive.
            * \param O* the data to read.
            * \param D... used for SFINAE.
            */
            template <class O>
            void operator() (std::vector<O>& objects) {
                std::size_t size;
                buffer>>size;
                char space;
                buffer.get(space);
                //////std::cout<<"vector size : "<<size<<std::endl;
                for (unsigned int i = 0; i < size; i++) {
                    O object;
                    (*this)(object);
                    objects.push_back(std::move(object));
                }
            }
            /**
            * \fn void operator() (std::unique_ptr<T>& ptr)
            * \brief read an std::unique_ptr from the archive.
            * \param std::unique_ptr<T>& the ptr to read.
            */
            template <class T>
            void operator() (std::unique_ptr<T>& ptr) {
                T* tmp;
                (*this)(tmp);
                if (tmp != nullptr)
                    ptr.reset(tmp);
            }
            /**
            * \fn void operator() (std::pair<T1, T2>& ptr)
            * \brief read an std::pair from the archive.
            * \param std::pair<T1, T2>& the std::pair to read.
            */
            template <class T1, class T2>
            void operator()(std::pair<T1, T2>& pair) {
                T1 type1;
                T2 type2;
                (*this)(type1);
                (*this)(type2);
                pair = std::make_pair(type1, type2);
            }
            /**
            * \fn void operator() (std::map<T1, T2>& ptr)
            * \brief read an std::map<T1, T2> from the archive.
            * \param std::map<T1, T2>& the std::pair to read.
            */
            template <class T1, class T2>
            void operator()(std::map<T1, T2>& map) {
                std::size_t size;
                buffer>>size;
                char space;
                buffer.get(space);
                //////std::cout<<"map size : "<<size<<std::endl;
                for (unsigned int i = 0; i < size; i++) {
                    std::pair<T1, T2> pair;
                    (*this)(pair);
                    map.insert(pair);
                }
            }
        private :
            std::istream& buffer; /**< the buffer where to read the data.*/
            std::map<long long int, std::string> adresses; /**< an std::map used to store ids and adresses of readed pointers.*/
            unsigned long long int nbDeserialized; /** the nb object which have been deserailized.*/
        };
    }
}
#endif // ODFAEG_ARCHIVE
