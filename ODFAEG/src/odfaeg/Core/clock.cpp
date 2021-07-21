#include "../../../include/odfaeg/Core/clock.h"
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
            Time elapsed;
            elapsed.time = std::chrono::high_resolution_clock::time_point(end.time_since_epoch() - start.time_since_epoch());
            return elapsed;
        }
    }
}
