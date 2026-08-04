#ifndef ODFAEG_STATEMANAGER_HPP
#define ODFAEG_STATEMANAGER_HPP
#include <string>
#include <vector>
#include <stdexcept>
#include "any.hpp"
namespace odfaeg {
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
            template <typename T> StateParameter(T value, std::string name);
            /**  \fn T getValue ()
            *    \brief get the value of the parameter.
            *    \return T : the value of the parameter.
            */
            template <typename T> T getValue() const;
            /**  \fn void setValue (T value)
            *    \brief set the value of a parameter.
            *    \return the value of the parameter.
            */
            template <typename T>  void setValue(T value);
            /** \fn std::string getName() const
            *   \return the name of the parameter.
            */
            std::string getName() const;
            /** \fn void setName()
            *   \brief std::string name : set the name of a parameter.
            *   \param std::string name : the name of the parameter.
            */
            void setName(std::string name);
            /** \fn ~StateParameter()
            *   \brief destructor.
            */
            ~StateParameter();
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
            StateGroup(std::string name);
            int getNbStatesGroup();
            bool execute();
            bool executeContrary();
            std::string getName();

            void addState(State* state);
            void removeState(std::string name);

            State* getState(std::string name);
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
            StateStack();
            void addStateGroup(StateGroup* stateGroup);

            void undo();
            void redo();
            StateGroup* getStateGroup(std::string name);
            ~StateStack();
        };        
    }
}
#endif