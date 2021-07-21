#ifndef ODFAEG_CLOCK_HPP
#define ODFAEG_CLOCK_HPP
#include <chrono>
#include "time.h"
namespace odfaeg {
    namespace core {
        /**
        * \file clock.hpp
        * \class Clock
        * \brief use stl classes to measure time.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        class Clock {
            public :
                /**
                * \fn Clock()
                * \brief initialize the clock.
                */
                Clock();
                /**
                * \fn restart()
                * \brief restart the clock.
                */
                void restart();
                /**
                * \fn Time getElapsedTime()
                * \brief get the elapsed time since the last call to restart.
                */
                Time getElapsedTime();
            private :
                std::chrono::high_resolution_clock::time_point start; /**> the high resolution clock.*/
        };
    }
}
#endif
