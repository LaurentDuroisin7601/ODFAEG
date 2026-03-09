#include "../../../include/odfaeg/Core/threadPool.hpp"
namespace odfaeg {
    namespace core {
        ThreadPool::ThreadPool (std::size_t threadCount) {
            for (std::size_t i = 0; i < threadCount; i++) {
                workers.emplace_back([this] {
                                        while(!stop) {
                                            std::function<void()> task;
                                            {
                                                std::unique_lock<std::mutex> lock(queueMutex);
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
        void ThreadPool::enqueue(std::function<void()> f)  {
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                tasks.emplace(f);
            }
            cv.notify_one();
        }
        ThreadPool::~ThreadPool() {
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                stop = true;
            }
            cv.notify_all();
            for (auto& t : workers) {
                t.join();
            }
        }
    }
}
