#include "../../../include/odfaeg/Core/time.h"
namespace odfaeg {
    namespace core {
        const Time Time::zero = Time();
        Time seconds (long int amount) {
            Time time;
            time.time = std::chrono::time_point<std::chrono::high_resolution_clock>(std::chrono::seconds(amount));
            return time;
        }
        Time milliseconds (sf::Int32 amount) {
            Time time;
            time.time = std::chrono::time_point<std::chrono::high_resolution_clock>(std::chrono::milliseconds(amount));
            return time;
        }
        Time microseconds (sf::Int64 amount) {
            Time time;
            time.time = std::chrono::time_point<std::chrono::high_resolution_clock>(std::chrono::microseconds(amount));
            return time;
        }
        Time::Time() {
            time = std::chrono::time_point<std::chrono::high_resolution_clock>(std::chrono::microseconds(0));
        }
        float Time::asSeconds() {
            const std::chrono::high_resolution_clock::duration since_epoch = time.time_since_epoch();
            return std::chrono::duration_cast<std::chrono::seconds>(since_epoch).count();
        }
        sf::Int32 Time::asMilliseconds() {
            const std::chrono::high_resolution_clock::duration since_epoch = time.time_since_epoch();
            return std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch).count();
        }
        sf::Int64 Time::asMicroseconds() {
            const std::chrono::high_resolution_clock::duration since_epoch = time.time_since_epoch();
            return std::chrono::duration_cast<std::chrono::microseconds>(since_epoch).count();
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
            Time time;
            time.time = std::chrono::high_resolution_clock::time_point(-right.time.time_since_epoch());
            return time;
        }
        Time operator+ (Time left, Time right) {
            Time time;
            time.time = std::chrono::high_resolution_clock::time_point(right.time.time_since_epoch() + left.time.time_since_epoch());
            return time;
        }
        Time& operator+= (Time& left, Time right) {
            left.time = std::chrono::high_resolution_clock::time_point(right.time.time_since_epoch() + left.time.time_since_epoch());
            return left;
        }
        Time operator- (Time left, Time right) {
            Time time;
            time.time = std::chrono::high_resolution_clock::time_point(left.time.time_since_epoch() - right.time.time_since_epoch());
            return time;
        }
        Time& operator-= (Time& left, Time right) {
            left.time = std::chrono::high_resolution_clock::time_point(left.time.time_since_epoch() - right.time.time_since_epoch());
            return left;
        }
        Time operator* (Time left, sf::Int64 real) {
            Time time;
            time.time = std::chrono::high_resolution_clock::time_point(left.time.time_since_epoch() * real);
            return time;
        }
        Time& operator*= (Time& left, sf::Int64 real) {
            left.time = std::chrono::high_resolution_clock::time_point(left.time.time_since_epoch() * real);
            return left;
        }
        float operator/ (Time left, Time right) {
            return left.time.time_since_epoch() / right.time.time_since_epoch();
        }
        Time operator/ (Time left, sf::Int64 real) {
            Time time;
            time.time = std::chrono::high_resolution_clock::time_point(left.time.time_since_epoch() / real);
            return time;
        }
        Time& operator/= (Time& left, sf::Int64 real) {
            left.time = std::chrono::high_resolution_clock::time_point(left.time.time_since_epoch() / real);
            return left;
        }
        Time operator% (Time left, sf::Int64 real) {
            Time time;
            time.time = std::chrono::high_resolution_clock::time_point(left.time.time_since_epoch() % real);
            return time;
        }
        Time& operator%= (Time& left, sf::Int64 real) {
            left.time = std::chrono::high_resolution_clock::time_point(left.time.time_since_epoch() % real);
            return left;
        }
    }
}
