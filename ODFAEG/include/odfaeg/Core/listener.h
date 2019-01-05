#ifndef ODFAEG_OBJECT_HPP
#define ODFAEG_OBJECT_HPP
#include "erreur.h"
#include "command.h"
#include <thread>
#include <map>
#include <mutex>
#include <condition_variable>
#include "export.hpp"
#include "singleton.h"
#include <chrono>
/**
 *\namespace odfaeg
 * the namespace of the Opensource Development Framework Adapted for Every Games.
 */
namespace odfaeg {
    namespace core {
        class ODFAEG_CORE_API Listener {
        /**
        * \file listener.h
        * \class Listener
        * \brief this class connect one or more command to an id, get the sf::Event and check if the command is triggered.
        * this class use a thread to execute the commands, and wait until we call a methode to check if the commands are trigered.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        public :
            /**
            * \fn Listener()
            * \brief constructor.
            */
             Listener() {

             }
             /**
             * \fn void connect (std::string key, Command command)
             * \brief connect an identifier to a command.
             * \param std:string key : the key to add.
             * \param Command command : the command to add.
             */
             void connect(std::string key, Command command) {
                toAdd.insert(std::make_pair(key, command));
             }
             /**
             * \fn void setCommandParams (std::string key,
             * \brief connect an identifier to a command.
             * \param std:string key : the key to add.
             * \param Command command : the command to add.
             */
             template <typename...A>
             void setCommandSigParams (std::string key, A&&... args) {

                 std::map<std::string, Command>::iterator it = commands.find(key);
                 if (it != commands.end())
                    it->second.setSigParams(std::forward<A>(args)...);
             }
             template <typename...A>
             void setCommandSlotParams (std::string key, A&&... args) {

                 std::map<std::string, Command>::iterator it = commands.find(key);
                 if (it != commands.end())
                    it->second.setSlotParams(std::forward<A>(args)...);
             }
             template <typename...A>
             void bindCommandSigParams (std::string key, A&&... args) {
                 std::map<std::string, Command>::iterator it = commands.find(key);
                 if (it != commands.end())
                    it->second.bindSigParams(std::forward<A>(args)...);
             }
             template <typename...A>
             void bindCommandSlotParams (std::string key, A&&... args) {
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
             /** \fn void pushEvent (sf::Event event)
             *   \brief push an event into the event stack.
             *   \param sf::Event : the sf::event to pass into the stack.
             */
             void pushEvent(sf::Event event) {
                 std::map<std::string, Command>::iterator it;
                 for (it = commands.begin(); it != commands.end(); it++) {
                    if(it->second.containsBufferEvent(event)) {
                        Command::pushEvent(event);
                    }
                    Action* action = it->second.getAction();
                    if(action != nullptr) {
                        action->setPressed(event);
                    }
                 }
             }
             /** \fn void processEvents ()
             *   \brief check if the commands are triggered and execute them.
             *   the current thread is blocked until this method is finished.
             */
             void processEvents() {
                 std::map<std::string, Command>::iterator it;
                 for (unsigned int i = 0; i < toRemove.size(); i++) {
                    it = commands.find(toRemove[i]);
                    if (it != commands.end()) {
                        commands.erase(it);
                    }
                 }
                 toRemove.clear();
                 for (it = toAdd.begin(); it != toAdd.end(); it++) {
                    commands.insert(std::make_pair(it->first, it->second));
                 }
                 toAdd.clear();
                 for (it = commands.begin(); it != commands.end(); it++) {
                    if (it->second.isTriggered()) {
                        (it->second)();
                    }
                    Action* action = it->second.getAction();
                    if (action != nullptr) {
                        std::vector<sf::Event> events;
                        action->getEvents(events);
                        for (unsigned int i = 0; i < events.size(); i++) {
                            Command::removeEvent(events[i]);
                        }
                    }
                 }
             }
             void removeLater(std::string name) {
                toRemove.push_back(name);
             }
             void removeCommand(std::string name) {
                std::map<std::string, Command>::iterator it;
                it = commands.find(name);
                if (it != commands.end()) {
                    commands.erase(it);
                }
             }
             private :
             /** \fn void stopListen()
             *   \brief stop the thread which triggers and execute commands.
             */
             std::map<std::string, Command> commands; /**> stores and execute commands.*/
             std::vector<std::string> toRemove;
             std::map<std::string, Command> toAdd;
        };
    }
}
#endif