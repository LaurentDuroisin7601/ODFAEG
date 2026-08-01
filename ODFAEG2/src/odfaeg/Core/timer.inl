namespace odfaeg {
    namespace core {
        /**
        * \file timer.h
        * \class Timer
        * \brief Each timer inherits from this class : a timer updates the scene with the given time's interval.
        * this class use a thread.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */        
        Timer::Timer(bool usingThread) : isUsingThread(usingThread), interval(seconds(0.1f)) {
            
        }
        void Timer::start() {
            if (isUsingThread) {                    
                m_thread = std::thread(&Timer::tUpdate, this);
            }
        }
        /**
            *  \fn setInterval(core::Time interval)
            *  \brief set an interval of time.
            *  \param core::Time interval : the time interval between two updates.
            */
        void Timer::setInterval(Time interval) {
            this->interval = interval;
        }
        /** \fn void run()
        *   \brief lock the mutex and updates the scene at each time interval.
        */
        void Timer::update() {
            ////////std::cout<<"update"<<std::endl;
            /*Time elapsedTime = clock.getElapsedTime();
            if (elapsedTime >= interval) {*/
                onUpdate();
                /*clock.restart();
            }*/
        }
        void Timer::tUpdate() {
            running.store(true);
            while (running.load()) {
                ////////std::cout<<"update"<<std::endl;
                //Time elapsedTime = clock.getElapsedTime();
                //if (!isRunning()) {
                    onUpdate();
                    //clock.restart();
                //}
            }
        }
        void Timer::stop() {
            if (isUsingThread.load() && running.load()) {
                running.store(false);
                cv.notify_all();
                //std::cout<<"join"<<std::endl;
                if (m_thread.joinable()) {
                    m_thread.join();
                    //std::cout<<"joined"<<std::endl;
                }
            }
        }
        /** \fn virtual void onUpdate() = 0;
        *   \brief the function to redefine when updating the scene.
        */
        void Timer::setName(std::string name) {
            this->name = name;
        }
        std::string Timer::getName() {
            return name;
        }            
        Timer::~Timer() {
            stop();
        }
        Time Timer::getElapsedTime() {
            return clock.getElapsedTime();
        }
        void Timer::restart() {
            clock.restart();
        }
        bool Timer::isRunning() {
            return running.load();
        }
        Time Timer::getInterval() {
            return interval;
        }
    }
}