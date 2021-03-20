/*
* @Author       : wangshuxiao
* @Date         : 2021-03-14
* @copyleft Apache 2.0
*/
#ifndef TIMEHEAP_H
#define TIMEHEAP_H

#include <time.h>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <arpa/inet.h>
#include <functional>
#include <assert.h>
#include <chrono>
#include <vector>

#include "../log/log.h"

using TimeoutCallBack = std::function<void()>;
using Clock = std::chrono::high_resolution_clock;
using MS = std::chrono::milliseconds;
using TimeStamp = Clock::time_point;

struct TimeNode {
    int id;
    TimeStamp expires;
    TimeoutCallBack cb;
    bool operator< (const TimeNode& t) {
        return expires < t.expires;
    }
};

class TimeHeap {
public:
    TimeHeap() { root_.reserve(64);}
    ~TimeHeap() { Clear(); }

    void Adjust(int id, int newExpires);
    void Add(int id, int timeOut, const TimeoutCallBack& cb);
    void DoWork(int id);

    void Clear();
    void Tick();

    int GetNextTick();

private:
    void Delete_(size_t i);
    void ShiftUp_(size_t i);
    bool ShiftDown_(size_t i, size_t n);
    void SwapNode_(size_t i, size_t j);

    std::vector<TimerNode> root_;
    std::unordered_map<int, size_t> ref_;                                       // id to position
};


#endif //TIMEHEAP_H
