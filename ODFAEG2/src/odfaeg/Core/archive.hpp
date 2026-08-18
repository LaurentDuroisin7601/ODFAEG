#ifndef ODFAEG_ARCHIVE_HPP
#define ODFAEG_ARCHIVE_HPP 
#include <vector>
#include <map>
#include <iostream>
#include <typeinfo>
#include <sstream>
#include <memory>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "serialization.hpp"
#include "utilities.hpp"
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
            static const bool value = sizeof(test<T>(0)) == sizeof(yes); /**> if the class has a typedef named KEYTYPE.*/
        };
		template <typename T> 
		concept IsFundamental = std::is_fundamental<T>::value;
		template <typename T>
		concept IsEnum = std::is_enum<T>::value;
		template <typename T>
		concept IsString = std::is_same<T, std::string>::value || std::is_same<T, const std::string>::value;
        template <typename T>
		concept IsDynamicObject = has_typedef_key<T>::value;
        /** \file archive.h
        * \class OTextArchive
        * \brief Write everything into the output archive's buffer in text format.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        class OTextArchive {
        public:
            /**\fn OTextArchive (std::ostream& buffer)
            *  \brief pass a c++ ouptut buffer to the archive.
            *  \param std::ostream& buffer : the output buffer.
            */
            OTextArchive(std::ostream& buffer);
            /** \fn bool isInputArchive()
            *   \brief check if the archive is an input archive.
            *   \return false because it's an output archive.
            */
            bool isInputArchive();
            /** \fn void clear()
            *   \brief clear the output stream buffer and the registered pointer adresses.
            */
            void clear();
            //Fundamentals.
            /**
            * \fn void operator(T& data, D...)
            * \brief write a fundamental type into the archive.
            * \param T& the data to write.
            * \param D... used for SFINAE.
            */
            template <typename T>
            void operator() (T& data) requires IsFundamental<T>;
            //Fundamentals.
            /**
            * \fn void operator(std::reference_wrapper<T> data, D...)
            * \brief write a fundamental type into the archive.
            * \param std::reference_wrapper<T> reference to the data to write.
            * \param D... used for SFINAE.
            */
            template <typename T>
            void operator() (std::reference_wrapper<T> ref) requires IsFundamental<T>;
            /**
            * \fn void operator(T* data, D...)
            * \brief write pointer to a fundamental type into the archive.
            * \param T* the pointer to write.
            * \param D... used for SFINAE.
            */
            template <typename T>
            void operator() (T* data) requires IsFundamental<T>;
            /** \fn void operator() (E& data, D...)
            *   \brief write an enum value to the archive.
            *   \param E& data : the data to write.
            */
            template <typename E>
            void operator() (E& data) requires IsEnum<E>;
            /** \fn void operator() (E& data, D...)
            *   \brief write an enum value to the archive.
            *   \param std::reference_wrapper<E> ref : the reference to the data to write.
            */
            template <typename E>
            void operator() (std::reference_wrapper<E> ref) requires IsEnum<E>;
            /** \fn void operator() (E& data, D...)
            *   \brief write an enum value to the archive.
            *   \param E* data : pointer to the data to write.
            */
            template <typename E>
            void operator() (E* data) requires IsEnum<E>;
            //std::string.
            /**
            *\fn void operator(T& data, D...)
            *\brief write an std::string into the archive.
            *\param T& data : the std::string to write.
            *\param D... : used fo SFINAE.
            */
            template <typename T>
            void operator() (T & data) requires IsString<T>;
            /**
            *\fn void operator(T& data, D...)
            *\brief write an std::string into the archive.
            *\param std::reference_wrapper<T> : the reference to the std::string to write.
            *\param D... : used fo SFINAE.
            */
            template <typename T>
            void operator() (std::reference_wrapper<T> ref) requires IsString<T>;
            /**
            *\fn void operator(T* data, D...)
            *\brief The pointer to the std::string to write.
            *\param T* data : the pointer to the data to write.
            *\param D... : used for SFINAE.
            */
            template <typename T>
            void operator() (T* data) requires IsString<T>;
            //Static objects.
            /**
            *\fn void operator(O& data, D...)
            *\brief register a static object onto the archive.
            *\param O& the object to register.
            *\param D... : used for SFINAE.
            */
            template <class O>
            void operator() (O& object) requires (!IsFundamental<O>) && (!IsDynamicObject<O>);
            //Static objects.
           /**
           *\fn void operator(O& data, D...)
           *\brief register a static object onto the archive.
           *\param std::reference_wrapper<O> the reference to the object to register.
           *\param D... : used for SFINAE.
           */
            template <class O>
            void operator() (std::reference_wrapper<O> ref) requires (!IsDynamicObject<O>);
            /**
            *\fn void operator(O* data, D...)
            *\brief register a static object onto the archive.
            *\param O* the pointer to the object to register.
            *\param D... : used for SFINAE.
            */
            template <class O>
            void operator() (O* object) requires (!IsDynamicObject<O>);
            //Dynamic objects.
            /**
            *\fn void operator(O& data, D...)
            *\brief register a dynamic object onto the archive.
            *\param O& the object to register.
            *\param D... : used for SFINAE.
            */
            /*template <class O>
            void operator() (O& object) requires (!IsFundamental<O> && IsDynamicObject<O>);*/
            /**
            *\fn void operator(O& data, D...)
            *\brief register a dynamic object onto the archive.
            *\param std::reference_wrapper<O> the reference to the object to register.
            *\param D... : used for SFINAE.
            */
            template <class O>
            void operator() (std::reference_wrapper<O> ref) requires (!IsFundamental<O> && IsDynamicObject<O>);
            /**
            *\fn void operator(O& data, D...)
            *\brief register pointer to a dynamic object onto the archive.
            *\param O& the object to register.
            *\param D... : used for SFINAE.
            */
            template <class O>
            void operator() (O* object) requires (!IsFundamental<O> && IsDynamicObject<O>);
            //std::vectors.
            /**
            *\fn void operator(std::vector<O>&, D...)
            *\brief register a list of objects onto the archive.
            *\param std::vector<O>& the list of objects to register.
            *\param D... : used for SFINAE.
            */
            template <class O>
            void operator() (std::vector<O>& data);
            /**
            *\fn void operator(std::unique_ptr<T>&, D...)
            *\brief register a std::unique_ptr onto the archive.
            *\param std::unique_ptr<T>& the unique pointer.
            *\param D... : used for SFINAE.
            */
            template <class T>
            void operator() (std::unique_ptr<T>& ptr);
            /**
            *\fn void operator(std::pair<T1, T2>&, D...)
            *\brief register an std::pair onto the archive.
            *\param std::pair<T1, T2>& the std::pair.
            *\param D... : used for SFINAE.
            */
            template <class T1, class T2>
            void operator() (std::pair<T1, T2>& pair);
            /**
            *\fn void operator(std::map<T1, T2>&, D...)
            *\brief register an std::map onto the archive.
            *\param std::map<T1, T2>& the std::map.
            *\param D... : used for SFINAE.
            */
            template <class T1, class T2>
            void operator() (std::map<T1, T2>& map);
        private:
            std::ostream& buffer; /**< the output buffer containing the datas.*/
            std::map<std::string, long long int> adresses; /**< an std::map used to store the adresses and the id of the serialized pointers.*/
            unsigned long long int nbSerialized; /** <the number data which are serialized into the archive*/
        };
		template <typename T>
		concept IsAbstractClass = std::is_abstract<T>::value;
        /**
        * \file archive.h
        * \class ITextArchive
        * \brief Read everything from the input archive's buffer.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        class ITextArchive {
        public:
            /**
            *\fn ITextArchive(std::istream& buffer)
            *\brief pass the input stream to the input test archive.
            *\param std::istream& buffer : the input buffer where to read the datas.
            */
            ITextArchive(std::istream& buffer);
            /**
            * \fn bool isAtEndOfInputStream()
            * \brief check if all the data has been extracted from the archive.
            * \return if all the data has been extracted from the archive.
            */
            bool isAtEndOfInputStream();
            /**
            *\fn bool isInputArchive()
            * \brief check if the archive is an input archive.
            * \return true because this is an input archive.
            */
            bool isInputArchive();
            /**
            * \fn void clear()
            * \brief clear the input stream buffer, and registered addresses.
            */
            void clear();
            //Fundamentals.
            /**
            * \fn void operator(T& data, D...)
            * \brief read a fundamental type from the archive.
            * \param T& the data to read.
            * \param D... used for SFINAE.
            */
            template <typename T>
            void operator() (T& data) requires IsFundamental<T>;
            //Fundamentals.
            /**
            * \fn void operator(T& data, D...)
            * \brief read a fundamental type from the archive.
            * \param std::reference_wrapper<T> reference to the data to read.
            * \param D... used for SFINAE.
            */
            template <typename T>
            void operator() (std::reference_wrapper<T> ref) requires IsFundamental<T>;
            /**
            * \fn void operator(T& data, D...)
            * \brief read a char from the archive. (we need to read unformatted input here to also read special chars like \n, spaces, etc...)
            * \param T& the data to read.
            * \param D... used for SFINAE.
            */

            void operator() (char& data);
            /**
            * \fn void operator(T& data, D...)
            * \brief read a char from the archive. (we need to read unformatted input here to also read special chars like \n, spaces, etc...)
            * \param std::reference_wrapper<char> reference to the data to read.
            * \param D... used for SFINAE.
            */

            void operator() (std::reference_wrapper<char> ref);
            /**
            * \fn void operator(T& data, D...)
            * \brief read an unsigned char from the archive. (we need to read unformatted input here to also read special chars like \n, spaces, etc...)
            * \param T& the data to read.
            * \param D... used for SFINAE.
            */

            void operator() (unsigned char& data);
            /**
            * \fn void operator(T& data, D...)
            * \brief read an unsigned char from the archive. (we need to read unformatted input here to also read special chars like \n, spaces, etc...)
            * \param std::reference_wrapper<unsigned char> reference the data to read.
            * \param D... used for SFINAE.
            */

            void operator() (std::reference_wrapper<unsigned char> ref);
            /**
            * \fn void operator(T& data, D...)
            * \brief read a pointer to a fundamental type from the archive.
            * \param T& the data to read.
            * \param D... used for SFINAE.
            */
            template <typename T>
            void operator() (T*& data) requires IsFundamental<T>;
            /**
           * \fn void operator(T& data, D...)
           * \brief read a pointer to a fundamental type from the archive.
           * \param T& the data to read.
           * \param D... used for SFINAE.
           */
            
            void operator() (char*& data);
            /**
           * \fn void operator(T& data, D...)
           * \brief read a pointer to a fundamental type from the archive.
           * \param T& the data to read.
           * \param D... used for SFINAE.
           */
            void operator() (unsigned char*& data);
            /**
            * \fn void operator()(E& data, D...)
            * \brief read an enum value from the archive.
            * \param E& the data the read.
            */
            template <typename E>
            void operator()(E& data) requires IsEnum<E>;
            /**
            * \fn void operator()(E& data, D...)
            * \brief read an enum value from the archive.
            * \param std::reference_wrapper<E> the reference to the data the read.
            */
            template <typename E>
            void operator()(std::reference_wrapper<E> ref) requires IsEnum<E>;
            template <typename E>
            void operator() (E*& data) requires IsEnum<E>;
            //std::string.
            /**
            * \fn void operator(T& data, D...)
            * \brief read an std::string from the archive.
            * \param T& the data to read.
            * \param D... used for SFINAE.
            */
            template <typename T>
            void operator() (T& data) requires IsString<T>;
            /**
           * \fn void operator(T& data, D...)
           * \brief read an std::string from the archive.
           * \param T& the data to read.
           * \param D... used for SFINAE.
           */
            template <typename T>
            void operator() (std::reference_wrapper<T> ref) requires IsString<T>;
            /**
            * \fn void operator(T& data, D...)
            * \brief read a pointer to an std::string from the archive.
            * \param T* the data to read.
            * \param D... used for SFINAE.
            */
            template <typename T>
            void operator() (T*& data) requires IsString<T>;
            //Static objects.
            /**
            * \fn void operator(O& data, D...)
            * \brief read a static object from the archive.
            * \param O& the data to read.
            * \param D... used for SFINAE.
            */
            template <class O>
            void operator() (O& object) requires (!IsFundamental<O> && !IsDynamicObject<O>);
            /**
            * \fn void operator(O& data, D...)
            * \brief read a static object from the archive.
            * \param std::reference_wrapper<O> the reference to the data to read.
            * \param D... used for SFINAE.
            */
            template <class O>
            void operator() (std::reference_wrapper<O> ref) requires (!IsDynamicObject<O>);
            /**
            * \fn void operator(O& data, D...)
            * \brief read a pointer to a static object from the archive.
            * \param O* the data to read.
            * \param D... used for SFINAE.
            */
            template <class O>
            void operator() (O*& object) requires (!IsDynamicObject<O>);
            //Dynamic objects.
            /**
            * \fn void operator(O* data, D...)
            * \brief read a dynamic object from the archive.
            * \param O& the data to read.
            * \param D... used for SFINAE.
            */
            template <class O>
            void operator() (O& object) requires (!IsFundamental<O> && IsDynamicObject<O>);
            /**
            * \fn void operator(O* data, D...)
            * \brief read a dynamic object from the archive.
            * \param std::reference_wrapper<O> the reference to the data to read.
            * \param D... used for SFINAE.
            */
            template <class O>
            void operator() (std::reference_wrapper<O> ref) requires (!IsFundamental<O> && IsDynamicObject<O>);
            /**
            * \fn void operator(O* data, D...)
            * \brief read a pointer to a non abstract dynamic object from the archive.
            * \param O* the data to read.
            * \param D... used for SFINAE.
            */
            template <class O>
            void operator() (O*& object) requires (!IsFundamental<O> && IsDynamicObject<O> && !IsAbstractClass<O>);
            /**
            * \fn void operator(O* data, D...)
            * \brief read a pointer to an abstract dynamic object from the archive.
            * \param O* the data to read.
            * \param D... used for SFINAE.
            */
            template <class O>
            void operator() (O*& object) requires (IsDynamicObject<O> && IsAbstractClass<O>);
            /**
            * \fn void operator(O* data, D...)
            * \brief read a list of objects from the archive.
            * \param O* the data to read.
            * \param D... used for SFINAE.
            */
            template <class O>
            void operator() (std::vector<O>& objects);
            /**
            * \fn void operator() (std::unique_ptr<T>& ptr)
            * \brief read an std::unique_ptr from the archive.
            * \param std::unique_ptr<T>& the ptr to read.
            */
            template <class T>
            void operator() (std::unique_ptr<T>& ptr);
            /**
            * \fn void operator() (std::pair<T1, T2>& ptr)
            * \brief read an std::pair from the archive.
            * \param std::pair<T1, T2>& the std::pair to read.
            */
            template <class T1, class T2>
            void operator()(std::pair<T1, T2>& pair);
            /**
            * \fn void operator() (std::map<T1, T2>& ptr)
            * \brief read an std::map<T1, T2> from the archive.
            * \param std::map<T1, T2>& the std::pair to read.
            */
            template <class T1, class T2>
            void operator()(std::map<T1, T2>& map);
        private:
            std::istream& buffer; /**< the buffer where to read the data.*/
            std::map<long long int, std::string> adresses; /**< an std::map used to store ids and adresses of readed pointers.*/
            unsigned long long int nbDeserialized; /** the nb object which have been deserailized.*/
        };
    }
}
#include "archive.inl"
#endif

