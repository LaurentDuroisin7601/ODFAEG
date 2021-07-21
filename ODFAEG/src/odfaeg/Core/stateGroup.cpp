#include "../../../include/odfaeg/Core/stateGroup.h"
using namespace std;
namespace odfaeg {
    namespace core {
        int StateGroup::nbStatesGroup = 0;
        StateGroup::StateGroup(string name) : name(name)
        {
            //ctor
            nbStatesGroup++;
        }
        int StateGroup::getNbStatesGroup () {
            return nbStatesGroup;
        }
        bool StateGroup::execute () {

            for (unsigned int i = 0; i < States.size(); i++) {
                if (!States[i]->doActionState())
                    return false;
            }
            return true;
        }
        bool StateGroup::executeContrary () {


            for (unsigned int i = 0; i < States.size(); i++) {

                if (!States[i]->undoActionState())
                    return false;

            }

            return true;
        }
        std::string StateGroup::getName () {
            return name;
        }

        void StateGroup::addState(State* state) {
             States.push_back(state);
        }
        void StateGroup::removeState (std::string name) {
            vector<State*>::iterator it;
            for (it = States.begin(); it != States.end(); it++) {
                if ((*it)->getName() == name) {
                    delete *it;
                    it = States.erase(it);
                }
            }
        }

        State* StateGroup::getState (string name) {
            for (unsigned int i= 0; i < States.size(); i++) {
                if (States[i]->getName() == name)
                    return States[i];
            }
            return nullptr;
        }
    }
}

