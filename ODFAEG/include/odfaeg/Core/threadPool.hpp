#ifndef ODFAEG_THREAD_POOL_HPP
#define ODFAEG_THREAD_POOL_HPP
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
        public :
            ThreadPool(std::size_t threadCount);
            ~ThreadPool();
            void enqueue(std::function<void()> f);
        private :
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

            void reset(int count) {
                remaining.store(count, std::memory_order_relaxed);
            }

            void jobDone() {
                if (remaining.fetch_sub(1, std::memory_order_acq_rel) == 1) {

                    std::lock_guard<std::mutex> lock(mtx);
                    cv.notify_one();
                }
            }
            void wait() {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [&] {return remaining.load() == 0;});
            }
        };
    }
}
#endif // ODFAEG_THREAD_POOL_HPP
