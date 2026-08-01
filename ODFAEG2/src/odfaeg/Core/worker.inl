namespace odfaeg {
    namespace core {        
        Worker::Worker(bool usingThread) : isUsingThread(usingThread), m_needToUpdate(false) {
            if (usingThread) {
                m_thread = std::thread(&Worker::tUpdate, this);
            }
        }
        void Worker::needToUpdate() {
            m_needToUpdate = true;
            //std::cout<<"update : "<<m_needToUpdate<<std::endl;
        }
        void Worker::update() {
            onUpdate();
        }
        void Worker::tUpdate() {
            running = true;
            while (running.load()) {
                //std::cout<<"update : "<<m_needToUpdate<<std::endl;
                if (m_needToUpdate.load()) {
                    //std::cout<<"update : "<<std::endl;
                    onUpdate();
                    m_needToUpdate = false;
                }
            }
        }
        void Worker::stop() {
            if (isUsingThread && running.load()) {
                running = false;
                m_thread.join();
            }
        }
        void Worker::setName(std::string name) {
            this->name = name;
        }
        std::string Worker::getName() {
            return name;
        }
        Worker::~Worker() {
            stop();
        }            
    }
}