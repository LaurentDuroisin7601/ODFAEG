#ifndef ODFAEG_STATE_EXECUTOR
#define ODFAEG_STATE_EXECUTOR
#include "state.h"
/**
 *\namespace odfaeg
 * the namespace of the Opensource Development Framework Adapted for Every Games.
 */
namespace odfaeg {
     namespace core {
        /**
        * \file stateExecutor.h
        * \class StateExecutor
        * \brief interface to apply or unapply a state.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        class ODFAEG_CORE_API StateExecutor {

                public :
                /**\fn bool doState(State& state)
                *  \brief function to redefine to apply a state.
                *  \param State& state : the state
                */
                virtual bool doState (State &state)  = 0;
                /**\fn bool undoState(State& state)
                *  \brief function to redefine to unapply a state.
                *  \param State& state : the state.
                */
                virtual bool undoState (State &state) = 0;
        };
    }
}
#endif // STATE_MANAGER
