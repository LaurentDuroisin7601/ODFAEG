module;
#include <chrono>
export module odfaeg.core.clock;
export namespace odfaeg {
    namespace core {
        class Time {
        public:
            friend class Clock;
            friend Time seconds(float amount);
            friend Time milliseconds(std::int32_t amount);
            friend Time microseconds(std::int64_t amount);
            friend bool 	operator== (Time left, Time right);
            friend bool 	operator!= (Time left, Time right);
            friend bool 	operator< (Time left, Time right);
            friend bool 	operator> (Time left, Time right);
            friend bool 	operator<= (Time left, Time right);
            friend bool 	operator>= (Time left, Time right);
            friend Time 	operator- (Time right);
            friend Time 	operator+ (Time left, Time right);
            friend Time& operator+= (Time& left, Time right);
            friend Time 	operator- (Time left, Time right);
            friend Time& operator-= (Time& left, Time right);
            friend Time 	operator* (Time left, std::int64_t right);
            friend Time& operator*= (Time& left, std::int64_t right);
            friend float    operator/ (Time left, Time right);
            friend Time 	operator/ (Time left, std::int64_t right);
            friend Time& operator/= (Time& left, std::int64_t right);
            friend Time 	operator% (Time left, std::int64_t right);
            friend Time& operator%= (Time& left, std::int64_t right);
            Time() {
                time = std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::duration<float>>{};
            }
            float asSeconds() {
                return time.time_since_epoch().count();
            }
            std::int32_t asMilliseconds() {
                return time.time_since_epoch().count() * 1000.f;
            }
            std::int64_t asMicroseconds() {
                return time.time_since_epoch().count() * 1'000'000.f;
            }
            static const Time zero;
        private:
            std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::duration<float>> time;
        };
        Time seconds(float amount) {
            Time time;
            time.time = std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::duration<float>>(std::chrono::duration<float>(amount));
            return time;
        }
        Time microseconds(std::int64_t amount) {
            Time time;
            time.time = std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::duration<float>>(std::chrono::duration<float>(amount / 1'000'000.f));
            return time;
        }
        Time milliseconds(std::int32_t amount)
        {
            Time time;
            time.time = std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::duration<float>>(std::chrono::duration<float>(amount / 1000.));
            return time;
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
        const Time Time::zero = Time();
        /**
        * \file clock.hpp
        * \class Clock
        * \brief use stl classes to measure time.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        class Clock {
        public:            
            Clock() {
                start = std::chrono::high_resolution_clock::now();
            }
            void restart() {
                start = std::chrono::high_resolution_clock::now();
            }
            Time getElapsedTime() {
                std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<float> elapsedSeconds = end - start;


                Time elapsed;
                elapsed.time = std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::duration<float>>(elapsedSeconds);

                return elapsed;
            }
        private:
            std::chrono::high_resolution_clock::time_point start; /**> the high resolution clock.*/
        };
    }
}