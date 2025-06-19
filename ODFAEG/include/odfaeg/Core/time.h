#ifndef ODFAEG_TIME_HPP
#define ODFAEG_TIME_HPP
#include <chrono>
#include "export.hpp"
namespace odfaeg {
    namespace core {
        class ODFAEG_CORE_API Time {
            public :
                friend class Clock;
                friend Time seconds (float amount);
                friend Time milliseconds (std::int32_t amount);
                friend Time microseconds (std::int64_t amount);
                friend bool 	operator== (Time left, Time right);
                friend bool 	operator!= (Time left, Time right);
                friend bool 	operator< (Time left, Time right);
                friend bool 	operator> (Time left, Time right);
                friend bool 	operator<= (Time left, Time right);
                friend bool 	operator>= (Time left, Time right);
                friend Time 	operator- (Time right);
                friend Time 	operator+ (Time left, Time right);
                friend Time & 	operator+= (Time &left, Time right);
                friend Time 	operator- (Time left, Time right);
                friend Time & 	operator-= (Time &left, Time right);
                friend Time 	operator* (Time left, std::int64_t right);
                friend Time 	operator* (std::int64_t left, Time right);
                friend Time & 	operator*= (Time &left, std::int64_t right);
                friend float    operator/ (Time left, Time right);
                friend Time 	operator/ (Time left, std::int64_t right);
                friend Time & 	operator/= (Time &left, std::int64_t right);
                friend Time 	operator% (Time left, std::int64_t right);
                friend Time 	operator% (std::int64_t left, Time right);
                friend Time & 	operator%= (Time &left, std::int64_t right);
                Time();
                float asSeconds();
                std::int32_t asMilliseconds();
                std::int64_t asMicroseconds();
                static const Time zero;
            private :
                std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::duration<float>> time;
        };
        Time seconds (float amount);
        Time milliseconds (std::int32_t amount);
        Time microseconds (std::int64_t amount);
        bool 	operator== (Time left, Time right);
        bool 	operator!= (Time left, Time right);
        bool 	operator< (Time left, Time right);
        bool 	operator> (Time left, Time right);
        bool 	operator<= (Time left, Time right);
        bool 	operator>= (Time left, Time right);
        Time 	operator- (Time right);
        Time 	operator+ (Time left, Time right);
        Time & 	operator+= (Time &left, Time right);
        Time 	operator- (Time left, Time right);
        Time & 	operator-= (Time &left, Time right);
        Time 	operator* (Time left, std::int64_t right);
        Time 	operator* (std::int64_t left, Time right);
        Time & 	operator*= (Time &left, std::int64_t right);
        Time 	operator/ (Time left, std::int64_t right);
        float   operator/ (Time left, Time right);
        Time 	operator/ (std::int64_t left, Time right);
        Time & 	operator/= (Time &left, std::int64_t right);
        Time 	operator% (Time left, std::int64_t right);
        Time 	operator% (std::int64_t left, Time right);
        Time & 	operator%= (Time &left, std::int64_t right);
    }
}
#endif
