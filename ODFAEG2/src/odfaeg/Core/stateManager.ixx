module;
#include <string>
#include <vector>
#include <stdexcept>
export module odfaeg.core.stateManager;
import odfaeg.core.any;
export namespace odfaeg {
    namespace core {
        class StateParameterBase {
        public:

            virtual std::string getName() const = 0;
            virtual void setName(std::string name) = 0;
        };
        /**
        * \file stateParameter.h
        * \class StateManager
        * \brief A parameter, this class is used to set parameters to the states.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        class StateParameter {

        private:
            Any value; /**> the value of the parameter. (can be of any type)*/
            std::string name; /**> the name of the parameter.*/

        public:
            /** \fn StateParameter (T value, std::string name)
            *    \brief constructor.
            *    \param T value : the value of the parameter.
            *    \param std::string name : the name of the parameter.
            */
            template <typename T> StateParameter(T value, std::string name) :
                name(name), value(std::forward<T>(value)) {
            }
            /**  \fn T getValue ()
            *    \brief get the value of the parameter.
            *    \return T : the value of the parameter.
            */
            template <typename T> T getValue() const {
                return value.get<T>();
            }
            /**  \fn void setValue (T value)
            *    \brief set the value of a parameter.
            *    \return the value of the parameter.
            */
            template <typename T>  void setValue(T value) {
                this->value.set(std::forward<T>(value));
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
		class StateExecutor;
        /**
        * \file state.h
        * \class State
        * \brief holds a particular state of the application. (Or of an entity)
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        class State {


        private:
            std::string name; /**> std::string name the name of the state*/
            StateExecutor* stateExec; /**> a state executor to apply and unapply the state.*/
            /** \fn State& operator= (const State &p);
            *   \brief affector
            *   \param const State &p : affect the current state.
            *   \return State& the affected state.
            */
            std::vector<StateParameter*> parameters; /**> parameters of the states.*/
            inline static int nbStates = 0; /**> nb states created. */
        public:
            State(std::string name, StateExecutor* exec);


            bool removeParameter(std::string name);
            bool doActionState();
            bool undoActionState();
            std::string getName() const;
            void setName(std::string name);
            /**\fn void setExecutor (StateExecutor *exec)
            *  \brief set the executor to apply or unapply the state.
            *  \param StateExecutor *exec : the executor.
            */
            void setExecutor(StateExecutor* exec);
            template <typename T> bool addParameter(std::string name, T value);

            StateParameter& getParameter(const std::string name);
            /**\fn  bool changeParameter (std::string name, T* value)
            *  \brief change the value of a parameter.
            *  \param std::string name : the name of the parameter.
            *  \param T* value : the value of the parameter.
            */
            template <typename T> void changeParameter(const std::string name, T value);

            static int getNbStates();
            /**\fn ~State()
            *  \brief destructor.
            */
            ~State();
        };
        /**
        * \file stateGroup.h
        * \class StateGroup
        * \brief this class stores a group of states.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        class StateGroup
        {
        public:
            StateGroup(std::string name) : name(name)
            {
                //ctor
                nbStatesGroup++;
            }
            int getNbStatesGroup() {
                return nbStatesGroup;
            }
            bool execute() {

                for (unsigned int i = 0; i < States.size(); i++) {
                    if (!States[i]->doActionState())
                        return false;
                }
                return true;
            }
            bool executeContrary() {


                for (unsigned int i = 0; i < States.size(); i++) {

                    if (!States[i]->undoActionState())
                        return false;

                }

                return true;
            }
            std::string getName() {
                return name;
            }

            void addState(State* state) {
                States.push_back(state);
            }
            void removeState(std::string name) {
                std::vector<State*>::iterator it;
                for (it = States.begin(); it != States.end(); it++) {
                    if ((*it)->getName() == name) {
                        delete* it;
                        it = States.erase(it);
                    }
                }
            }

            State* getState(std::string name) {
                for (unsigned int i = 0; i < States.size(); i++) {
                    if (States[i]->getName() == name)
                        return States[i];
                }
                return nullptr;
            }        
        private:
            inline static int nbStatesGroup = 0; /**>the number of the states stores.*/
            std::vector<State*> States; /**> the list of the states stored.*/
            std::string name; /**>the name of the states.*/
        };
        /**
        * \file stateExecutor.h
        * \class StateExecutor
        * \brief interface to apply or unapply a state.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        class StateExecutor {

        public:
            /**\fn bool doState(State& state)
            *  \brief function to redefine to apply a state.
            *  \param State& state : the state
            */
            virtual bool doState(State& state) = 0;
            /**\fn bool undoState(State& state)
            *  \brief function to redefine to unapply a state.
            *  \param State& state : the state.
            */
            virtual bool undoState(State& state) = 0;
        };
        class StateStack {
            static const int STACK_CAPACITY = 20;
        private:
            StateGroup* States[STACK_CAPACITY];
            int currentStateId;
        public:
            StateStack() {
                currentStateId = -1;
                for (int i = 0; i < STACK_CAPACITY; i++) {
                    States[i] = nullptr;
                }
            }
            void addStateGroup(StateGroup* stateGroup) {
                currentStateId++;
                for (unsigned int i = currentStateId; i < STACK_CAPACITY; i++) {
                    if (States[currentStateId] != nullptr) {
                        delete States[i];
                        States[i] = nullptr;
                    }
                }
                if (currentStateId >= STACK_CAPACITY) {
                    delete States[0];
                    for (unsigned int i = 1; i < STACK_CAPACITY; i++) {
                        States[i - 1] = States[i];
                    }
                    currentStateId = STACK_CAPACITY - 1;
                }
                States[currentStateId] = stateGroup;
            }

            void undo() {

                if (currentStateId < 0) {
                    return;
                }
                if (States[currentStateId]->executeContrary()) {
                    currentStateId--;
                }
            }
            void redo() {
                currentStateId++;

                if (States[currentStateId] != NULL && currentStateId < STACK_CAPACITY) {
                    States[currentStateId]->execute();
                }
                else {
                    currentStateId--;
                }
            }
            StateGroup* getStateGroup(std::string name) {
                for (int i = 0; i < STACK_CAPACITY; i++) {
                    if (States[i] != nullptr && States[i]->getName() == name) {
                        return States[i];
                    }
                }
                return nullptr;
            }
            ~StateStack() {
                for (int i = 0; i < STACK_CAPACITY; i++) {
                    if (States[i] != NULL)
                        delete States[i];
                }
            }
        };
        /**\fn  State (std::string name, StateExecutor *stateExec);
            *  \brief create a state with the given state executor which apply and unapply the state.
            *  \param std::string name : the name of the state.
            *  \param StateExecutor stateExec : a pointer to a StateExecutor which apply and unapply a state.
            */
        State::State(std::string name, StateExecutor* exec) : name(name), stateExec(exec) {
            nbStates++;
        }


        bool State::removeParameter(std::string name) {
            std::vector<StateParameter*>::iterator it;
            for (it = parameters.begin(); it != parameters.end();) {
                if ((*it)->getName() == name) {
                    delete* it;
                    it = parameters.erase(it);
                    return true;
                }
                else {
                    it++;
                }
            }
            return false;
        }
        bool State::doActionState() {
            if (stateExec->doState(*this)) {
                return true;
            }
            else
                return false;

        }
        bool State::undoActionState() {
            if (stateExec->undoState(*this))
                return true;
            else
                return false;
        }
        std::string State::getName() const {
            return name;
        }
        void State::setName(std::string name) {
            this->name = name;
        }
        /**\fn void setExecutor (StateExecutor *exec)
        *  \brief set the executor to apply or unapply the state.
        *  \param StateExecutor *exec : the executor.
        */
        void State::setExecutor(StateExecutor* exec) {
            this->stateExec = exec;
        }
        template <typename T> bool State::addParameter(std::string name, T value) {
            StateParameter* p = new StateParameter(value, name);

            for (unsigned int i = 0; i < parameters.size(); i++) {
                if (parameters[i]->getName() == name) {
                    delete p;
                    return false;
                }
            }

            parameters.push_back(p);
            return true;
        }

        StateParameter& State::getParameter(const std::string name) {
            for (unsigned int i = 0; i < parameters.size(); i++) {
                if (parameters[i]->getName() == name) {

                    return *parameters[i];
                }
            }
            throw std::runtime_error("No such parameter for this State : " + name);
        }
        /**\fn  bool changeParameter (std::string name, T* value)
        *  \brief change the value of a parameter.
        *  \param std::string name : the name of the parameter.
        *  \param T* value : the value of the parameter.
        */
        template <typename T> void State::changeParameter(const std::string name, T value) {
            StateParameter& parameter = getParameter(name);
            parameter.setValue(value);
        }

        int State::getNbStates() {
            return nbStates;
        }
        /**\fn ~State()
        *  \brief destructor.
        */
        State::~State() {
            for (unsigned int i = 0; i < parameters.size(); i++) {
                delete parameters[i];
            }
            parameters.clear();
            nbStates--;
        }
    }
}