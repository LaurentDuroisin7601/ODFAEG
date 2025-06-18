#ifndef STATE_MANAGER
#define STATE_MANAGER
/**
 *\namespace odfaeg
 * the namespace of the Opensource Development Framework Adapted for Every Games.
 */
namespace odfaeg {
    namespace core {
        /**
        * \file stateManager.h
        * \class StateManager
        * \brief holds a list of states.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        class ODFAEG_CORE_API StateManager {
        public :
            /**\fn StateManager ()
            *  \brief constructor
            */
            StateManager () {}
            /**\fn pushState(State &state)
            *  \brief push a state into the list.
            *  \param State& state : the state to store.
            */
            void pushState (State *state) {
                states.push_back(state);
                currentState = states.back();
            }
            /**\fn popState()
            *  \brief pop a state into the list.
            */
            void popState () {
                states.pop_back();
                currentState = states.back();
            }
            /**\fn  State& getCurrentState ()
            *  \brief get the last state of the list.
            *  \return the last state.
            */
            State* getCurrentState () {
                return currentState;
            }
        private :
            std::vector<State*> states; /**>the list of the states.*/
            State* currentState; /**>the current state.*/
        };
    }
}
#endif // STATE_MANAGER
