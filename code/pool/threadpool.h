/*
 * @Author       : wangshuxiao
 * @Date         : 2021-03-13
 * @copyleft Apache 2.0
 */


#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>
#include <memory>

class ThreadPool {
public:
    explicit ThreadPool(size_t threadCount = 8):pool_(std::make_shared<Pool>()) {
        assert(threadCount > 0);
        for (size_t i = 0; i < threadCount; i++) {
            std::thread([pool = pool_] {
                std::unique_lock<std::mutex> locker (pool->mtx);
                while(true) {
                    if (!pool->task.empty()) {
                        auto task = std::move(pool->tasks.front());
                        pool->tasks.pop();
                        locker.unlock();
                        task();
                        locker.lock();
                    }
                    else if (pool->isClosed) break;
                    // condiction to get the locker
                    else pool->cond.wait(locker);
                }
            }).detach();
        }
    }

    ThreadPool() = default;

    ThreadPool(ThreadPool &&) = default;

    ~ThreadPool() {
        if (static_cast<bool> (pool_)) {
            {
                std::lock_guard<std::mutex> locker(pool_->mtx);
                pool_->isClosed = true;
            }
        }
        pool_->cond.notify_all();
    }

    template<class F>
    void AddTask(F&& Task) {
        {
            std::lock_guard<std::mutex> locker(pool_->mtx);
            pool_->tasks.emplace(std::forward<F>(task));
        }
        pool_->cond.notify_one();
    }

private:
    struct Pool{
        std::mutex mtx;
        std::condition_variable cond;
        bool isClose;
        std::queue<std::function<void()> > tasks;
    };
    std::shared_ptr<Pool>pool_;
};

#endif //THREADPOOL_H
