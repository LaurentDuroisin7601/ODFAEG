#ifndef ODFAEG_ANY_HPP
#define ODFAEG_ANY_HPP
#include <typeinfo>
#include "erreur.h"
#include "export.hpp"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace core {
        /**
        * \file any.h
        * \class value_base
        * \brief base class used for the type erasure.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        class ODFAEG_CORE_API value_base {
        public :
            /** \fn value_case* clone() const = 0;
            *   \brief clone the value.
            *   \return value_base : a pointer to the cloned object.
            */
            virtual value_base* clone() const = 0;
            virtual ~value_base () { }
        };
        /**
        * \file any.h
        * \class value
        * \brief Store a value of any type. (Used by the state system)
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        * Store a value of any type.
        */
        template <typename T> class value : public value_base {
        public :
            friend class any;
            /**\fn value(const T& t)
            *  \brief store a value of any type.
            *  \param T : the value.
            */
            value (const T& t) : t(t) { }
            /**\fn value_base* clone ()
            *  \brief clone the object.
            *  \return value_base* : a pointer to the cloned object.
            */
            value_base* clone() const
            {
                return new value(t);
            }
        private :
            /**< The value*/
            T t;
        };
        /**
        * \file any.h
        * \class any
        * \brief set and get a value of any type. (Used by the state system)
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        class ODFAEG_CORE_API any {
        public :
            /**\fn any ()
            *  \brief default constructor. (No value)
            */
            any() : v(0) { }
            /**\fn any (T& v)
            *  \brief constructor. (make an object containing a value of any type)
            *  \param the value.
            */
            template <typename T>
            any (const T& v_) : v(new value<T>(v_)), type(const_cast<std::type_info*>(&typeid(v_))) { }
             /**\fn any (T& v)
            *  \brief copy constructor.
            *  \param any& object : the object to copy.
            */
            any(any const & other) : v(other.v ? other.v->clone() : 0) {}
            /**\fn operator= (T& v)
            *  \brief affector.
            *  \param any& object : the object to copy.
            *  \return any& : the copied object.
            */
            any& operator=(const any& other)
            {
                if(&other != this)
                {
                    any copy(other);
                    swap(copy);
                }
                return *this;
            }
             /**\fn swap (any& other)
            *  \brief exchange the content of two any objects.
            *  \param any& other : the object to swap content which.
            */
            void swap(any& other)
            {
                std::swap(v, other.v);
            }
            /**\fn get ()
            *  \brief get the value of the any object.
            *  \return T : the value of the any object.
            */
            template <typename T>
            T& get () const {
                value<T>* v = dynamic_cast<value<T>*> (this->v);
                if (v == 0) {
                    throw Erreur (0, "Bad cast exception.", 0);
                } else {
                    return v->t;
                }
            }
            /**\fn set ()
            *  \brief set the value of the any object.
            *  \param v : the value of the any object to set.
            */
            template <typename T> void set(T v_) {
                delete v;
                v = new value<T> (v_);
            }
            /**\fn std::type_info& getType ()
            *  \brief get the type of the value.
            *  \return std::type_info : the type of the value.
            */
            std::type_info& getType() {
                return *type;
            }
            /**\fn ~any()
            * \brief destructor.
            */
            ~any () { delete v; }
        private :
            value_base *v; /**< value_base : the object containing the value*/
            std::type_info* type; /**< type : the type of the value*/
        };
    }
}
#endif
