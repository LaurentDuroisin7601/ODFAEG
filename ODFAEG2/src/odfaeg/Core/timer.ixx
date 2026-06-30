module;
#include <thread>
#include <iostream>
#include <condition_variable>
export module odfaeg.core.timer;
import odfaeg.core.clock;
/**
 *\namespace odfaeg
 * the namespace of the Opensource Development Framework Adapted for Every Games.
 */
export namespace odfaeg {
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
            Timer(bool usingThread=true) : isUsingThread(usingThread), interval(seconds(0.1f)) {
                
            }
            void start() {
                if (isUsingThread) {                    
                    m_thread = std::thread(&Timer::tUpdate, this);
                }
            }
            /**
                *  \fn setInterval(core::Time interval)
                *  \brief set an interval of time.
                *  \param core::Time interval : the time interval between two updates.
                */
            void setInterval(Time interval) {
                this->interval = interval;
            }
            /** \fn void run()
            *   \brief lock the mutex and updates the scene at each time interval.
            */
            void update() {
                ////////std::cout<<"update"<<std::endl;
                /*Time elapsedTime = clock.getElapsedTime();
                if (elapsedTime >= interval) {*/
                    onUpdate();
                    /*clock.restart();
                }*/
            }
            void tUpdate() {
                running.store(true);
                while (running.load()) {
                    ////////std::cout<<"update"<<std::endl;
                    //Time elapsedTime = clock.getElapsedTime();
                    //if (!isRunning()) {
                        onUpdate();
                        //clock.restart();
                    //}
                }
            }
            void stop() {
                if (isUsingThread.load() && running.load()) {
                    running.store(false);
                    cv.notify_all();
                    //std::cout<<"join"<<std::endl;
                    if (m_thread.joinable()) {
                        m_thread.join();
                        //std::cout<<"joined"<<std::endl;
                    }
                }
            }
            /** \fn virtual void onUpdate() = 0;
            *   \brief the function to redefine when updating the scene.
            */
            void setName(std::string name) {
                this->name = name;
            }
            std::string getName() {
                return name;
            }            
            ~Timer() {
                stop();
            }
            Time getElapsedTime() {
                return clock.getElapsedTime();
            }
            void restart() {
            clock.restart();
            }
            bool isRunning() {
                return running.load();
            }
            Time getInterval() {
                return interval;
            }
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