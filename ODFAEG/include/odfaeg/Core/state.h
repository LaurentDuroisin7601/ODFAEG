#ifndef State_H
#define State_H
#include <vector>
#include "erreur.h"
#include <sstream>
#include "stateParameter.h"
#include <memory>
#include <iostream>
/**
 *\namespace odfaeg
 * the namespace of the Opensource Development Framework Adapted for Every Games.
 */
namespace odfaeg {
        namespace core {
        class StateExecutor;
        /**
        * \file state.h
        * \class State
        * \brief holds a particular state of the application. (Or of an entity)
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        class ODFAEG_API_EXPORT State {


            private :
                std::string name; /**> std::string name the name of the state*/
                StateExecutor* stateExec; /**> a state executor to apply and unapply the state.*/
                /** \fn State& operator= (const State &p);
                *   \brief affector
                *   \param const State &p : affect the current state.
                *   \return State& the affected state.
                */
                std::vector<StateParameter*> parameters; /**> parameters of the states.*/
                static int nbStates; /**> nb states created. */
            public :
                /**\fn  State (std::string name, StateExecutor *stateExec);
                *  \brief create a state with the given state executor which apply and unapply the state.
                *  \param std::string name : the name of the state.
                *  \param StateExecutor stateExec : a pointer to a StateExecutor which apply and unapply a state.
                */
                State (std::string name, StateExecutor *stateExec);
                std::string getName () const;
                void setName (std::string name);
                template <typename T> bool addParameter (std::string name, T value) {
                    StateParameter* p = new StateParameter (value, name);

                    for (unsigned int i = 0; i < parameters.size(); i++) {
                        if (parameters[i]->getName() == name) {
                            delete p;
                            return false;
                        }
                    }

                    parameters.push_back(p);
                    return true;
                }
                /**\fn  bool removeParameter (std::string name)
                *  \brief remove a parameter to the state.
                *  \param std::string name : the name of the parameter.
                */
                bool removeParameter (std::string name);
                StateParameter& getParameter (const std::string name) {
                        for (unsigned int i = 0; i < parameters.size(); i++) {
                            if (parameters[i]->getName() == name) {

                                return *parameters[i];
                            }
                        }
                        throw Erreur(5, "No such parameter for this State : " + name, 2);
                }
                /**\fn  bool changeParameter (std::string name, T* value)
                *  \brief change the value of a parameter.
                *  \param std::string name : the name of the parameter.
                *  \param T* value : the value of the parameter.
                */
                template <typename T> void changeParameter (const std::string name, T value) {
                    StateParameter& parameter = getParameter(name);
                    parameter.setValue(value);
                }
                /**\fn void setExecutor (StateExecutor *exec)
                *  \brief set the executor to apply or unapply the state.
                *  \param StateExecutor *exec : the executor.
                */
                void setExecutor (StateExecutor* exec) {
                    this->stateExec = exec;
                }
                /**\fn bool doActionState ()
                *  \brief apply the state.
                *  \return true if the state has been correctly applied.
                */
                bool doActionState ();
                /**\fn bool undoActionState ()
                *  \brief unapply the state.
                *  \return true if the state has been correctly applied.
                */
                bool undoActionState ();
                static int getNbStates () {
                    return nbStates;
                }
                /**\fn ~State()
                *  \brief destructor.
                */
                ~State () {
                    for (unsigned int i = 0; i < parameters.size(); i++) {
                        delete parameters[i];
                    }
                    parameters.clear();
                    nbStates--;
                }
        };
    }
}
#endif
