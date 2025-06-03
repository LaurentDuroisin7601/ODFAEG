#ifndef ODFAEG_ENTITY_SYSTEM_HPP
#define ODFAEG_ENTITY_SYSTEM_HPP
#include <condition_variable>
#include <thread>
#include "export.hpp"
#include <chrono>
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
        class ODFAEG_CORE_API EntitySystem {
        public :
            EntitySystem(bool useThread=true);
            /**
            *\fn void update()
            *\brief function called when we need to update the entities, this function block the current thread until it's finished.
            */
            void needToUpdate();
            void update ();
            void tUpdate();
            void setName(std::string name);
            std::string getName();
            void stop();
            /**
            *\fn void onUpdate()
            *\brief function to refefines to updates the entities.
            */
            virtual void onUpdate() = 0;
            virtual ~EntitySystem();
        private:
            std::string name;
            bool m_needToUpdate, running, isUsingThread;
            std::thread m_thread;
        };
    }
}
#endif // ENTITY_SYSTEM
