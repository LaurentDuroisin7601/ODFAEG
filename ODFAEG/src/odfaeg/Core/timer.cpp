#include "../../../include/odfaeg/Core/timer.h"

namespace odfaeg {
    namespace core {
        /**
            *  \fn setInterval(sf::Time interval)
            *  \brief set an interval of time.
            *  \param sf::Time interval : the time interval between two updates.
            */
            void Timer::setInterval(sf::Time interval) {
                this->interval = interval;
            }
            /** \fn void run()
            *   \brief lock the mutex and updates the scene at each time interval.
            */
            void Timer::update() {
                sf::Time elapsedTime = clock.getElapsedTime();
                if (elapsedTime >= interval) {
                    onUpdate();
                    clock.restart();
                }
            }
            /** \fn virtual void onUpdate() = 0;
            *   \brief the function to redefine when updating the scene.
            */
            void Timer::setName(std::string name) {
                this->name = name;
            }
            std::string Timer::getName() {
                return name;
            }
            Timer::~Timer() {}
    }
}
