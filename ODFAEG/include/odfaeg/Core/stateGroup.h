#ifndef StateGROUP_H
#define StateGROUP_H
#include <vector>
#include "state.h"
#include <string>
/**
 *\namespace odfaeg
 * the namespace of the Opensource Development Framework Adapted for Every Games.
 */
namespace odfaeg {
    namespace core {
        /**
        * \file stateGroup.h
        * \class StateGroup
        * \brief this class stores a group of states.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        class ODFAEG_CORE_API StateGroup
        {
            public:
                /**\fn StateGroup(std::string name)
                *  \brief create a group with the following name.
                *  \param std::string name : the name of the group.
                */
                StateGroup(std::string name);
                /**\fn void addState (State *a)
                *  \brief add the state to the group.
                *  \param State *a : the state to add.
                */
                void addState (State* a);
                /**\fn execute()
                *  \brief apply the states.
                *  \return true if the states are correctly applied.
                */
                bool execute ();
                /**\fn executeContrary()
                *  \brief unapply the states.
                *  \return true if the states are correctly unapplied.
                */
                bool executeContrary ();
                /**\fn removeState(std::string name)
                *  \brief remove a state from the group.
                *  \param the name of the state to remove.
                */
                void removeState (std::string name);
                /**\fn getState(std::string name)
                *  \brief get a state of the group.
                *  \param the name of the state to get.
                *  \return the state.
                */
                State* getState(std::string name);
                /**\fn std::string getName()
                *  \brief get the name of the group.
                *  \return the name of the group.
                */
                std::string getName ();
                /**\fn int getNbStatesGroup()
                *  \brief get the number of the states stored into the groups
                *  \return the number of the states stored.
                */
                static int getNbStatesGroup ();
                /*virtual ~StateGroup() {
                    for (unsigned int i = 0; i < States.size(); i++) {
                        delete States[i];
                    }
                    States.clear();

                }*/
            protected:
            private:
                static int nbStatesGroup; /**>the number of the states stores.*/
                std::vector<State*> States; /**> the list of the states stored.*/
                std::string name; /**>the name of the states.*/
        };
    }
}
#endif // StateGROUP_H
