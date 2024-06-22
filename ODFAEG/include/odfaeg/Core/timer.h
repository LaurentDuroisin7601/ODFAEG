#ifndef TIMER
#define TIMER
#include "singleton.h"
/**
 *\namespace odfaeg
 * the namespace of the Opensource Development Framework Adapted for Every Games.
 */
namespace odfaeg {
    namespace core {
        /**
        * \file timer.h
        * \class Timer
        * \brief Each timer inherits from this class : a timer updates the scene with the given time's interval.
        * this class use a thread.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        class ODFAEG_CORE_API Timer {
        public :
            /**
            *  \fn setInterval(sf::Time interval)
            *  \brief set an interval of time.
            *  \param sf::Time interval : the time interval between two updates.
            */
            void setInterval(sf::Time interval) {
                this->interval = interval;
            }
            /** \fn void run()
            *   \brief lock the mutex and updates the scene at each time interval.
            */
            void update() {
                sf::Time elapsedTime = clock.getElapsedTime();
                if (elapsedTime >= interval) {
                    onUpdate();
                    clock.restart();
                }
            }
            /** \fn virtual void onUpdate() = 0;
            *   \brief the function to redefine when updating the scene.
            */
            virtual void onUpdate() = 0;
            void setName(std::string name) {
                this->name = name;
            }
            std::string getName() {
                return name;
            }
        private :
            sf::Clock clock; /**> A clock use to measure the time elapsed since the last update*/
            sf::Time interval; /**> The time interval between two updates.*/
            std::string name;
        };
    }
}
#endif // TIMER
