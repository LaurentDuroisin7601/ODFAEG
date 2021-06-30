#ifndef ODFAEGCREATOR_STATE_EXECUTOR
#define ODFAEGCREATOR_STATE_EXECUTOR
#include "odfaeg/Core/stateExecutor.h"
class ODFAEGCreator;
class ODFAEGCreatorStateExecutor : public odfaeg::core::StateExecutor {
    public :
    ODFAEGCreatorStateExecutor(ODFAEGCreator* odfaegCreator);
    bool doState(odfaeg::core::State &state);
    bool undoState (odfaeg::core::State &state);
    private :
    ODFAEGCreator* app;
};
#endif
