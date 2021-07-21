#ifndef ODFAEG_TIME_HPP
#define ODFAEG_TIME_HPP
#include <SFML/System.hpp>
#include <chrono>
namespace odfaeg {
    namespace core {
        class Time {
            public :
                friend class Clock;
                friend Time seconds (long int amount);
                friend Time milliseconds (sf::Int32 amount);
                friend Time microseconds (sf::Int64 amount);
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
                friend Time 	operator* (Time left, sf::Int64 right);
                friend Time 	operator* (sf::Int64 left, Time right);
                friend Time & 	operator*= (Time &left, sf::Int64 right);
                friend float    operator/ (Time left, Time right);
                friend Time 	operator/ (Time left, sf::Int64 right);
                friend Time & 	operator/= (Time &left, sf::Int64 right);
                friend Time 	operator% (Time left, sf::Int64 right);
                friend Time 	operator% (sf::Int64 left, Time right);
                friend Time & 	operator%= (Time &left, sf::Int64 right);
                Time();
                float asSeconds();
                sf::Int32 asMilliseconds();
                sf::Int64 asMicroseconds();
                static const Time zero;
            private :
                std::chrono::high_resolution_clock::time_point time;
        };
        Time seconds (long int amount);
        Time millisconds (sf::Int32 amount);
        Time microseconds (sf::Int64 amount);
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
        Time 	operator* (Time left, sf::Int64 right);
        Time 	operator* (sf::Int64 left, Time right);
        Time & 	operator*= (Time &left, sf::Int64 right);
        Time 	operator/ (Time left, sf::Int64 right);
        float   operator/ (Time left, Time right);
        Time 	operator/ (sf::Int64 left, Time right);
        Time & 	operator/= (Time &left, sf::Int64 right);
        Time 	operator% (Time left, sf::Int64 right);
        Time 	operator% (sf::Int64 left, Time right);
        Time & 	operator%= (Time &left, sf::Int64 right);
    }
}
#endif
