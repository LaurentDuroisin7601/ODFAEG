 namespace odfaeg {
    namespace core {
        /** \fn StateParameter (T value, std::string name)
        *    \brief constructor.
        *    \param T value : the value of the parameter.
        *    \param std::string name : the name of the parameter.
        */
        template <typename T> StateParameter::StateParameter(T value, std::string name) :
            name(name), value(std::forward<T>(value)) {
        }
        /**  \fn T getValue ()
        *    \brief get the value of the parameter.
        *    \return T : the value of the parameter.
        */
        template <typename T> T StateParameter::getValue() const {
            return value.get<T>();
        }
        /**  \fn void setValue (T value)
        *    \brief set the value of a parameter.
        *    \return the value of the parameter.
        */
        template <typename T>  void StateParameter::setValue(T value) {
            this->value.set(std::forward<T>(value));
        }
        /** \fn std::string getName() const
        *   \return the name of the parameter.
        */
        std::string StateParameter::getName() const {
            return name;
        }
        /** \fn void setName()
        *   \brief std::string name : set the name of a parameter.
        *   \param std::string name : the name of the parameter.
        */
        void StateParameter::setName(std::string name) {
            this->name = name;
        }
        /** \fn ~StateParameter()
        *   \brief destructor.
        */
        StateParameter::~StateParameter() {

        }        		
        StateGroup::StateGroup(std::string name) : name(name)
        {
            //ctor
            nbStatesGroup++;
        }
        int StateGroup::getNbStatesGroup() {
            return nbStatesGroup;
        }
        bool StateGroup::execute() {

            for (unsigned int i = 0; i < States.size(); i++) {
                if (!States[i]->doActionState())
                    return false;
            }
            return true;
        }
        bool StateGroup::executeContrary() {


            for (unsigned int i = 0; i < States.size(); i++) {

                if (!States[i]->undoActionState())
                    return false;

            }

            return true;
        }
        std::string StateGroup::getName() {
            return name;
        }

        void StateGroup::addState(State* state) {
            States.push_back(state);
        }
        void StateGroup::removeState(std::string name) {
            std::vector<State*>::iterator it;
            for (it = States.begin(); it != States.end(); it++) {
                if ((*it)->getName() == name) {
                    delete* it;
                    it = States.erase(it);
                }
            }
        }

        State* StateGroup::getState(std::string name) {
            for (unsigned int i = 0; i < States.size(); i++) {
                if (States[i]->getName() == name)
                    return States[i];
            }
            return nullptr;
        }
        StateStack::StateStack() {
            currentStateId = -1;
            for (int i = 0; i < STACK_CAPACITY; i++) {
                States[i] = nullptr;
            }
        }
        void StateStack::addStateGroup(StateGroup* stateGroup) {
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

        void StateStack::undo() {

            if (currentStateId < 0) {
                return;
            }
            if (States[currentStateId]->executeContrary()) {
                currentStateId--;
            }
        }
        void StateStack::redo() {
            currentStateId++;

            if (States[currentStateId] != NULL && currentStateId < STACK_CAPACITY) {
                States[currentStateId]->execute();
            }
            else {
                currentStateId--;
            }
        }
        StateGroup* StateStack::getStateGroup(std::string name) {
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