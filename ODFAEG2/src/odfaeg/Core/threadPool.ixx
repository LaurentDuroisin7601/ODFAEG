module;
#include <thread>
#include <functional>
#include <condition_variable> 
#include <mutex>
#include <vector>
#include <queue>
#include <iostream>
export module odfaeg.core.threadPool;
export namespace odfaeg {
    namespace core {
        class ThreadPool {
        public:
            ThreadPool(std::size_t threadCount) {
                for (std::size_t i = 0; i < threadCount; i++) {
                    workers.emplace_back([this] {
                        while (!stop) {
                            std::unique_lock<std::mutex> lock(queueMutex);
                            std::function<void()> task;
                            {
                                
                                cv.wait(lock, [this] {
                                    return stop || !tasks.empty();
                                    });
                                if (stop && tasks.empty())
                                    return;
                                task = std::move(tasks.front());
                                tasks.pop();
                            }
                            task();
                        }
                        });
                }
            }
            void enqueue(std::function<void()> f) {
                {
                    std::lock_guard<std::mutex> lock(queueMutex);
                    tasks.emplace(f);
                }
                cv.notify_one();
            }
            ~ThreadPool() {
                {
                    std::lock_guard<std::mutex> lock(queueMutex);
                    stop = true;
                }
                cv.notify_all();
                for (auto& t : workers) {
                    t.join();
                }
            }
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
            JobFence()
            {
            }

            void reset(int count) {
                remaining.store(count, std::memory_order_relaxed);
            }

            void jobDone() {
                //std::cout<<"remaining : "<<remaining.load()<<std::endl;
                if (remaining.fetch_sub(1, std::memory_order_acq_rel) == 1) {

                    std::lock_guard<std::mutex> lock(mtx);
                    cv.notify_one();
                }
            }
            void wait() {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [&] {return remaining.load() == 0; });
            }
        };
    }
}