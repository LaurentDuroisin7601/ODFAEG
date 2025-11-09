#include "../../../include/odfaeg/Core/state.h"
#include "../../../include/odfaeg/Core/stateExecutor.h"
using namespace std;

namespace odfaeg {
    namespace core {
        int State::nbStates = 0;
        State::State (std::string name, StateExecutor *exec) : name(name), stateExec(exec) {
            nbStates++;
        }


        bool State::removeParameter (string name) {
            vector<StateParameter*>::iterator it;
            for (it = parameters.begin(); it != parameters.end();) {
                if ((*it)->getName() == name) {
                    delete *it;
                    it = parameters.erase(it);
                    return true;
                } else {
                    it++;
                }
            }
            return false;
        }
        bool State::doActionState () {
            if(stateExec->doState(*this)) {
                return true;
            } else
                return false;

        }
        bool State::undoActionState () {
            if (stateExec->undoState(*this))
                return true;
            else
                return false;
        }
        string State::getName () const {
            return name;
        }
        void State::setName (string name) {
            this->name = name;
        }
    }
}






