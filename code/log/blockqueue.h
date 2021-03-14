/*
 * @Author       : wangshuxiao
 * @Date         : 2021-03-14
 * @copyleft Apache 2.0
 */

#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include <sys/time.h>

template<class T>
class BlockQueue{
public:
    explicit BlockQueue(size_t maxCapacity = 1000);
    ~BlockQueue();

    void Clear();
    bool Empty();
    bool Full();
    void Close();
    size_t Size();
    size_t Capacity();

    T Front();
    T Back();

    void Push_back(const T& item);
    void Push_front(const T& item);
    bool Pop(T &item);
    bool Pop(T &itrm, int timeout);
    void Flush();

private:
    bool isClose_;
    std::deque<T>deq_;
    size_t capacity_;
    std::mutex mtx_;
    std::condition_variable condConsumer_;
    std::condition_variable conProducer_;
};
#endif //BLOCKQUEUE_H

template<class T>
BlockQueue<T>::BlockQueue(size_t maxCapacity):capacity_(maxCapacity) {
    assert(capacity_ > 0);
    isClose_ = false;
}

template<class T>
BlockQueue<T>::~BlockQueue() {
    Close();
}

template<class T>
void BlockQueue<T>::Close() {
    {
        std::lock_guard<std::mutex>locker(mtx_);
        deq_.clear()；
        isClose_ = true;
    }
    conProducer_.notify_all();
    condConsumer_.notify_all();
}

template<class T>
void BlockQueue<T>::Flush() {
    condConsumer_.notify_one();
};

template<class T>
void BlockQueue<T>::Clear() {
    std::lock_guard<std::mutex>locker(mtx_);
    deq_.clear();
}

template<class T>
T BlockQueue<T>::Front() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.front();
}

template<class T>
T BlockQueue<T>::Back() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.back();
}

template<class T>
T BlockQueue<T>::Size() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.size();
}

template<class T>
size_t BlockQueue<T>::Capacity() {
    std::lock_guard<std::mutex> locker(mtx_);
    return capacity_;
}

template<class T>
void BlockQueue<T>::Push_back(const T &item) {
    std::unique_lock<std::mutex>locker(mtx_);
    // to prevent from the fake wake up
    while(deq_.size() >= capacity_) {
        conProducer_.wait(locker);
    }
    deq_.push_back(item);
    condConsumer_.notify_one();
}

template<class T>
void BlockQueue<T>::Push_front(const T &item) {
    std::unique_lock<std::mutex>locker(mtx_);
    // to prevent from the fake wake up
    while(deq_.size() >= capacity_) {
        conProducer_.wait(locker);
    }
    deq_.push_front(item);
    condConsumer_.notify_one();
}

template<class T>
bool BlockQueue<T>::Empty() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.empty();
}

template<class T>
bool BlockQueue<T>::Full() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.size() >= capacity_;
}

template<class T>
bool BlockQueue<T>::Pop(T &item) {
    std::unique_lock<std::mutex>locker(mtx_);
    while(deq_.empty()) {
        condConsumer_.wait(locker);
        if(isClose_) return false;
    }
    item = deq_.front();
    deq_.pop_front();
    conProducer_.notify_one();
    return true;
}

template<class T>
bool BlockQueue<T>::Pop(T &itrm, int timeout) {
    std::unique_lock<std::mutex>locker(mtx_);
    while(deq_.empty()) {
        if (condConsumer_.template wait_for(locker, std::chrono::seconds(timeout))
                == std::cv_status::timeout) {
            return false;
        }
        if (isClose_) return false;
    }
    item = deq_.front();
    deq_.pop_front();
    conProducer_.notify_one();
    return true;
}










