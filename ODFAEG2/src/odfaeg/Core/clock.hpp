#include <chrono>
namespace odfaeg {
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
            Time();
            float asSeconds();
            std::int32_t asMilliseconds();
            std::int64_t asMicroseconds();
            static const Time zero;
        private:
            std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::duration<float>> time;
        }; 
        Time seconds(float amount);
        Time microseconds(std::int64_t amount);
        Time milliseconds(std::int32_t amount);
        bool operator== (Time left, Time right);
        bool operator!= (Time left, Time right);
        bool operator< (Time left, Time right);
        bool operator> (Time left, Time right);
        bool operator<= (Time left, Time right);
        bool operator>= (Time left, Time right);
        Time operator- (Time right);
        Time operator+ (Time left, Time right);
        Time& operator+= (Time& left, Time right);
        Time operator- (Time left, Time right);
        Time& operator-= (Time& left, Time right);
        Time operator* (Time left, std::int64_t real);
        Time& operator*= (Time& left, std::int64_t real);
        float operator/ (Time left, Time right);
        Time operator/ (Time left, std::int64_t real);
        Time& operator/= (Time& left, std::int64_t real);
        Time operator% (Time left, std::int64_t real);
        Time& operator%= (Time& left, std::int64_t real);
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
            Clock();
            void restart();
            Time getElapsedTime();
        private:
            std::chrono::high_resolution_clock::time_point start; /**> the high resolution clock.*/
        };
    }
}