module;
#include <condition_variable>
#include  <thread>
#include  <chrono>
export module odfaeg.core.worker;

/**
*\namespace odfaeg
* the namespace of the Opensource Development Framework Adapted for Every Games.
*/
export namespace odfaeg {
    namespace core {
        /**
        * \file entitySystem.h
        * \class EntitiesSystem
        * \brief base class of all entities systems of the odfaeg.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        class Worker {
        public:
            Worker(bool usingThread) : isUsingThread(usingThread), m_needToUpdate(false) {
                if (usingThread) {
                    m_thread = std::thread(&Worker::tUpdate, this);
                }
            }
            void needToUpdate() {
                m_needToUpdate = true;
                //std::cout<<"update : "<<m_needToUpdate<<std::endl;
            }
            void update() {
                onUpdate();
            }
            void tUpdate() {
                running = true;
                while (running.load()) {
                    //std::cout<<"update : "<<m_needToUpdate<<std::endl;
                    if (m_needToUpdate.load()) {
                        //std::cout<<"update : "<<std::endl;
                        onUpdate();
                        m_needToUpdate = false;
                    }
                }
            }
            void stop() {
                if (isUsingThread && running.load()) {
                    running = false;
                    m_thread.join();
                }
            }
            void setName(std::string name) {
                this->name = name;
            }
            std::string getName() {
                return name;
            }
            ~Worker() {
                stop();
            }
            virtual void onUpdate() = 0;            
            bool isUsingThread;
        private:
            std::string name;
            std::atomic<bool> m_needToUpdate, running;
            std::thread m_thread;
        };
    }
}

