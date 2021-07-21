#include "../../../include/odfaeg/Core/stateStack.h"
#include <iostream>
using namespace std;
namespace odfaeg {
    namespace core {
        StateStack::StateStack () {
            currentStateId = -1;
            for (int i = 0; i < STACK_CAPACITY; i++) {
                States[i] = nullptr;
            }
        }
        void StateStack::addStateGroup (StateGroup* stateGroup) {
            currentStateId++;
            for (unsigned int i = currentStateId; i < STACK_CAPACITY; i++) {
                if (States[currentStateId] != nullptr) {
                    delete States[i];
                    States[i] = nullptr;
                }
            }
            if (currentStateId >=  STACK_CAPACITY) {
                delete States[0];
                for (unsigned int i = 1; i < STACK_CAPACITY; i++) {
                    States[i - 1] = States[i];
                }
                currentStateId =  STACK_CAPACITY - 1;
            }
            States[currentStateId] = stateGroup;
        }

        void StateStack::undo () {

            if (currentStateId < 0) {
                return;
            }
            if (States[currentStateId]->executeContrary()) {
                currentStateId--;
            }
        }
        void StateStack::redo () {
            currentStateId++;

            if (States[currentStateId] != NULL && currentStateId < STACK_CAPACITY) {
                States[currentStateId]->execute();
            } else {
                currentStateId--;
            }
        }
        StateGroup* StateStack::getStateGroup (string name) {
            for (int i = 0; i < STACK_CAPACITY; i++) {
                if (States[i] != nullptr && States[i]->getName() == name) {
                    return States[i];
                }
            }
            return nullptr;
        }
        StateStack::~StateStack() {
            for (int i = 0; i < STACK_CAPACITY; i++) {
                if (States[i] != NULL)
                    delete States[i];
            }
        }
    }
}
