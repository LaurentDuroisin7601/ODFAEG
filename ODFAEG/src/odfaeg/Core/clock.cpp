#include "../../../include/odfaeg/Core/clock.h"
#include <iostream>
namespace odfaeg {
    namespace core {
        using namespace std::chrono;
        Clock::Clock() {
            start = high_resolution_clock::now();
        }
        void Clock::restart() {
            start = high_resolution_clock::now();
        }
        Time Clock::getElapsedTime() {
            std::chrono::high_resolution_clock::time_point end = high_resolution_clock::now();
            std::chrono::duration<float> elapsedSeconds = end - start;


            Time elapsed;
            elapsed.time = std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::duration<float>>(elapsedSeconds);

            return elapsed;
        }
    }
}
