#include <condition_variable>
#include  <thread>
#include  <chrono>
/**
*\namespace odfaeg
* the namespace of the Opensource Development Framework Adapted for Every Games.
*/
namespace odfaeg {
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
            Worker(bool usingThread);
            void needToUpdate();
            void update();
            void tUpdate();
            void stop();
            void setName(std::string name);
            std::string getName();
            ~Worker();
            virtual void onUpdate() = 0;            
            bool isUsingThread;
        private:
            std::string name;
            std::atomic<bool> m_needToUpdate, running;
            std::thread m_thread;
        };
    }
}
#include "worker.inl"

