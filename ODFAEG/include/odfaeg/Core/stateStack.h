#ifndef StateStack_H
#define StateStack_H
#include <vector>

#include <string>

#include "erreur.h"
#include "stateGroup.h"
namespace odfaeg {
    namespace core {
        class ODFAEG_CORE_API StateStack {
            static const int STACK_CAPACITY = 20;
            private :
                StateGroup* States[STACK_CAPACITY ];
                int currentStateId;
            public :
                StateStack();
                void addStateGroup (StateGroup* StateGroup);
                void undo ();
                void redo ();
                StateGroup* getStateGroup (std::string name);
                ~StateStack();
        };
    }
}
#endif
