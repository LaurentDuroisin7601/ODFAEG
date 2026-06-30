module;
#include <thread>
#include <map>
#include <condition_variable>
#include <chrono>
#include <iostream>
export module odfaeg.window.listener;
import odfaeg.window.iEvent;
import odfaeg.window.command;
import odfaeg.window.action;
/**
 *\namespace odfaeg
 * the namespace of the Opensource Development Framework Adapted for Every Games.
 */
namespace odfaeg {
    namespace window {
        export class Listener {
            /**
            * \file listener.h
            * \class Listener
            * \brief this class connect one or more command to an id, get the window::IEvent and check if the command is triggered.
            * this class use a thread to execute the commands, and wait until we call a methode to check if the commands are trigered.
            * \author Duroisin.L
            * \version 1.0
            * \date 1/02/2014
            */
        public:
            /**
            * \fn Listener()
            * \brief constructor.
            */
            std::string name;
            Listener() {
            }
            void launch() {
                //std::cout<<"load!"<<std::endl;
                if (!running.load()) {
                    useThread.store(true);
                    m_thread = std::thread(&Listener::tProcessEvents, this);
                }
            }
            bool isUsingThread() {
                return useThread.load();
            }
            /**
            * \fn void connect (std::string key, Command command)
            * \brief connect an identifier to a command.
            * \param std:string key : the key to add.
            * \param Command command : the command to add.
            */
            void connect(std::string key, Command command) {
                command.setName(key);
                commands.insert(std::make_pair(key, command));
            }
            void blockCommand(std::string key, bool blocked) {
                if (blocked) {                    
                    std::map<std::string, Command>::iterator it = commands.find(key);                    
                    if (it != commands.end()) {
                        bool found = false;
                        for (unsigned int i = 0; i < blockedCommands.size() && !found; i++) {
                            if (blockedCommands[i] == key) {
                                found = true;
                            }
                        }
                        if (!found) {
                            blockedCommands.push_back(key);
                        }
                    }
                }
                else {
                    std::vector<std::string>::iterator it;
                    for (it = blockedCommands.begin(); it != blockedCommands.end();) {
                        if (*it == key) {
                            it = blockedCommands.erase(it);
                        }
                        else {
                            it++;
                        }
                    }
                }
            }
            /**
            * \fn void setCommandParams (std::string key,
            * \brief connect an identifier to a command.
            * \param std:string key : the key to add.
            * \param Command command : the command to add.
            */
            template <typename...A>
            void setCommandSigParams(std::string key, A&&... args) {

                std::map<std::string, Command>::iterator it = commands.find(key);
                if (it != commands.end())
                    it->second.setSigParams(std::forward<A>(args)...);
            }
            template <typename...A>
            void setCommandSlotParams(std::string key, A&&... args) {

                std::map<std::string, Command>::iterator it = commands.find(key);
                if (it != commands.end())
                    it->second.setSlotParams(std::forward<A>(args)...);
            }
            /**
            * \fn void setCommandParams (std::string key,
            * \brief connect an identifier to a command.
            * \param std:string key : the key to add.
            * \param Command command : the command to add.
            */
            template <typename...A>
            void bindCommandSigParams(std::string key, A&&... args) {

                std::map<std::string, Command>::iterator it = commands.find(key);
                if (it != commands.end())
                    it->second.bindSigParams(std::forward<A>(args)...);
            }
            template <typename...A>
            void bindCommandSlotParams(std::string key, A&&... args) {

                std::map<std::string, Command>::iterator it = commands.find(key);
                if (it != commands.end())
                    it->second.bindSlotParams(std::forward<A>(args)...);
            }
            /** bool isOneTriggered()
            *   \fn bool isOneTriggered()
            *   \brief return true if a least one command is triggered
            *   \return if a command is triggered.
            */
            bool isOneTriggered() {
                std::map<std::string, Command>::iterator it;
                for (it = commands.begin(); it != commands.end(); it++) {
                    if (it->second.isTriggered()) {
                        return true;
                    }
                }
                return false;
            }
            /** \fn void pushEvent (window::IEvent event)
            *   \brief push an event into the event stack.
            *   \param window::IEvent : the window::IEvent to pass into the stack.
            */
            void pushEvent(IEvent event) {
                std::map<std::string, Command>::iterator it;
                for (it = commands.begin(); it != commands.end(); it++) {                    
                    Action* action = it->second.getAction();
                    if (action != nullptr) {
                        action->pushEvent(event);
                        action->setPressed(event);
                    }
                }
            }
            void stop() {
                if (useThread.load()) {
                    running.store(false);
                    if (m_thread.joinable())
                        m_thread.join();
                }
            }
            /** \fn void processEvents ()
            *   \brief check if the commands are triggered and execute them.
            *   the current thread is blocked until this method is finished.
            */
            void tProcessEvents() {
                running.store(true);
                while (running.load()) {
                    //std::cout<<"running!"<<std::endl;
                    std::map<std::string, Command>::iterator it;
                    for (unsigned int i = 0; i < toRemove.size(); i++) {
                        it = commands.find(toRemove[i]);
                        if (it != commands.end()) {
                            commands.erase(it);
                        }
                    }
                    toRemove.clear();                    
                    for (it = commands.begin(); it != commands.end(); it++) {                        
                        bool found = false;
                        for (unsigned int i = 0; i < blockedCommands.size() && !found; i++) {
                            if (it->first == blockedCommands[i])
                                found = true;
                        }
                        if (!found) {
                            if (it->second.isTriggered()) {
                                //std::cout<<"triggered"<<std::endl;
                                (it->second)();                                
                            }                            
                        }                       
                    }                    
                }
            }
            void processEvents() {
                std::map<std::string, Command>::iterator it;
                for (unsigned int i = 0; i < toRemove.size(); i++) {
                    it = commands.find(toRemove[i]);
                    if (it != commands.end()) {
                        commands.erase(it);
                    }
                }
                toRemove.clear();                
                for (it = commands.begin(); it != commands.end(); it++) {
                    bool found = false;
                    for (unsigned int i = 0; i < blockedCommands.size() && !found; i++) {
                        if (it->first == blockedCommands[i]) {
                            //std::cout<<"block command : "<<it->first<<std::endl;
                            found = true;
                        }
                    }
                    if (!found) {
                        /*if (it->second.getName() == "DropDownTextChanged")
                            std::cout<<"is icon moved triggered ? "<<std::endl;*/
                        if (it->second.isTriggered()) {
                            (it->second)();                            
                        }                        
                    }                    
                }                
            }
            void clearEventsStack() {
                std::map<std::string, Command>::iterator it;
                for (it = commands.begin(); it != commands.end(); it++) {
                    it->second.clearEventsStack();
                }                
            }
            void removeCommand(std::string name) {
                toRemove.push_back(name);
            }   
            ~Listener() {
                if (useThread)
                    stop();
            }
            bool isRunning() {
                return running;
            }
        private:
            /** \fn void stopListen()
            *   \brief stop the thread which triggers and execute commands.
            */
            std::map<std::string, Command> commands; /**> stores and execute commands.*/
            std::vector<std::string> toRemove;
            std::vector<std::string> blockedCommands;            
            std::atomic<bool> useThread, running;
            std::thread m_thread;
        };
    }
}