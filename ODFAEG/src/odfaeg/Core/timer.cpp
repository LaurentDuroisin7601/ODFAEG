#include "../../../include/odfaeg/Core/timer.h"
#include <iostream>
namespace odfaeg {
    namespace core {
            Timer::Timer(bool usingThread) : isUsingThread(usingThread), interval(seconds(0.1f)) {
                if (usingThread) {
                    m_thread = std::thread(&Timer::tUpdate, this);
                }
            }
        /**
            *  \fn setInterval(core::Time interval)
            *  \brief set an interval of time.
            *  \param core::Time interval : the time interval between two updates.
            */
            void Timer::setInterval(Time interval) {
                this->interval = interval;
            }
            /** \fn void run()
            *   \brief lock the mutex and updates the scene at each time interval.
            */
            void Timer::update() {
                ////////std::cout<<"update"<<std::endl;
                Time elapsedTime = clock.getElapsedTime();
                if (elapsedTime >= interval) {
                    onUpdate();
                    clock.restart();
                }
            }
            void Timer::tUpdate() {
                running = true;
                while (running) {
                    ////////std::cout<<"update"<<std::endl;
                    Time elapsedTime = clock.getElapsedTime();
                    if (elapsedTime >= interval) {
                        onUpdate();
                        clock.restart();
                    }
                }
            }
            void Timer::stop() {
                if (isUsingThread && running) {
                    running = false;
                    m_thread.join();
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
            Timer::~Timer() {
                stop();
            }
    }
}
