#ifndef UPS_State_PARAMETER_H
#define UPS_State_PARAMETER_H
#include "any.h"
#include <string>
/**
 *\namespace odfaeg
 * the namespace of the Opensource Development Framework Adapted for Every Games.
 */
namespace odfaeg {
    namespace core {
        /**
        * \file stateParameter.h
        * \class StateManager
        * \brief A parameter, this class is used to set parameters to the states.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        class ODFAEG_CORE_API StateParameter {

            private :
                any value; /**> the value of the parameter. (can be of any type)*/
                std::string name; /**> the name of the parameter.*/

            public :
                /** \fn StateParameter (T value, std::string name)
                *    \brief constructor.
                *    \param T value : the value of the parameter.
                *    \param std::string name : the name of the parameter.
                */
                template <typename T> StateParameter (T value, std::string name) :
                     name (name), value (std::forward<T>(value)) {
                }
                /**  \fn T getValue ()
                *    \brief get the value of the parameter.
                *    \return T : the value of the parameter.
                */
                template <typename T> T getValue () const {
                    return value.get<T>();
                }
                /**  \fn void setValue (T value)
                *    \brief set the value of a parameter.
                *    \return the value of the parameter.
                */
                template <typename T>  void setValue (T value) {
                    this->value.set (std::forward<T>(value));
                }
                /** \fn std::string getName() const
                *   \return the name of the parameter.
                */
                std::string getName() const {
                    return name;
                }
                /** \fn void setName()
                *   \brief std::string name : set the name of a parameter.
                *   \param std::string name : the name of the parameter.
                */
                void setName(std::string name) {
                    this->name = name;
                }
                /** \fn ~StateParameter()
                *   \brief destructor.
                */
                ~StateParameter() {

                }
        };
    }
}
#endif


