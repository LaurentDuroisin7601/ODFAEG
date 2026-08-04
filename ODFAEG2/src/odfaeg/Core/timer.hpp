#ifndef ODFAEG_TIMER_HPP
#define ODFAEG_TIMER_HPP
#include <thread>
#include <iostream>
#include <condition_variable>
#include "clock.hpp"
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
        class Timer {
        public:
            std::condition_variable cv;
            Timer(bool usingThread=true);
            void start();
            /**
                *  \fn setInterval(core::Time interval)
                *  \brief set an interval of time.
                *  \param core::Time interval : the time interval between two updates.
                */
            void setInterval(Time interval);
            /** \fn void run()
            *   \brief lock the mutex and updates the scene at each time interval.
            */
            void update();
            void tUpdate();
            void stop();
            /** \fn virtual void onUpdate() = 0;
            *   \brief the function to redefine when updating the scene.
            */
            void setName(std::string name);
            std::string getName();            
            ~Timer();
            Time getElapsedTime();
            void restart();
            bool isRunning();
            Time getInterval();
        protected:
			virtual void onUpdate() = 0;
        private:
            Clock clock; /**> A clock use to measure the time elapsed since the last update*/
            Time interval; /**> The time interval between two updates.*/
            std::string name;
            std::thread m_thread;
            std::atomic<bool> running, isUsingThread;
        };
    }
}
#endif