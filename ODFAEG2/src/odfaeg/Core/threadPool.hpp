#include <thread>
#include <functional>
#include <condition_variable> 
#include <mutex>
#include <vector>
#include <queue>
#include <iostream>
namespace odfaeg {
    namespace core {
        class ThreadPool {
        public:
            ThreadPool(std::size_t threadCount);
            void enqueue(std::function<void()> f);
            ~ThreadPool();
        private:
            std::vector<std::thread> workers;
            std::queue<std::function<void()>> tasks;
            std::mutex queueMutex;
            std::condition_variable cv;
            bool stop = false;
        };
        struct JobFence {
            std::atomic<int> remaining = 0;
            std::mutex mtx;
            std::condition_variable cv;
            JobFence();
            void reset(int count);

            void jobDone();
            void wait();
        };
    }
}
#include "threadPool.inl" 
