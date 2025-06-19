#include "../../../include/odfaeg/Core/time.h"
namespace odfaeg {
    namespace core {
        const Time Time::zero = Time();
        Time seconds (float amount) {
            Time time;
            time.time = std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::duration<float>>(std::chrono::duration<float>(amount));
            return time;
        }
        Time milliseconds (std::int32_t amount) {
            Time time;
            time.time = std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::duration<float>>(std::chrono::duration<float>(amount / 1000.));
            return time;
        }
        Time microseconds (std::int64_t amount) {
            Time time;
            time.time = std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::duration<float>>(std::chrono::duration<float>(amount / 1'000'000.f));
            return time;
        }
        Time::Time() {
            time = std::chrono::time_point<std::chrono::high_resolution_clock>(std::chrono::microseconds(0));
        }
        float Time::asSeconds() {
            return time.time_since_epoch().count();
        }
        std::int32_t Time::asMilliseconds() {
            return time.time_since_epoch().count() * 1000.f;
        }
        std::int64_t Time::asMicroseconds() {
            return time.time_since_epoch().count() * 1'000'000.f;
        }
        bool operator== (Time left, Time right) {
            return left.time == right.time;
        }
        bool operator!= (Time left, Time right) {
            return left.time != right.time;
        }
        bool operator< (Time left, Time right) {
            return left.time < right.time;
        }
        bool operator> (Time left, Time right) {
            return left.time > right.time;
        }
        bool operator<= (Time left, Time right) {
            return left.time <= right.time;
        }
        bool operator>= (Time left, Time right) {
            return left.time >= right.time;
        }
        Time operator- (Time right) {
            Time time = milliseconds(-right.asMilliseconds());
            return time;
        }
        Time operator+ (Time left, Time right) {
            Time time = milliseconds(right.asMilliseconds() + left.asMilliseconds());
            return time;
        }
        Time& operator+= (Time& left, Time right) {
            left = milliseconds(right.asMilliseconds() + left.asMilliseconds());
            return left;
        }
        Time operator- (Time left, Time right) {
            Time time = milliseconds(left.asMilliseconds() - right.asMilliseconds());
            return time;
        }
        Time& operator-= (Time& left, Time right) {
            left = milliseconds(left.asMilliseconds() - right.asMilliseconds());
            return left;
        }
        Time operator* (Time left, std::int64_t real) {
            Time time = milliseconds(left.asMilliseconds() * real);
            return time;
        }
        Time& operator*= (Time& left, std::int64_t real) {
            left = milliseconds(left.asMilliseconds() / real);
            return left;
        }
        float operator/ (Time left, Time right) {
            return left.asMilliseconds() / right.asMilliseconds();
        }
        Time operator/ (Time left, std::int64_t real) {
            Time time = milliseconds(left.asMilliseconds() / real);
            return time;
        }
        Time& operator/= (Time& left, std::int64_t real) {
            left = milliseconds(left.asMilliseconds() / real);
            return left;
        }
        Time operator% (Time left, std::int64_t real) {
            Time time = milliseconds(left.asMilliseconds() * real);
            return time;
        }
        Time& operator%= (Time& left, std::int64_t real) {
            left = milliseconds(left.asMilliseconds() % real);
            return left;
        }
    }
}
