/*
* @Author       : wangshuxiao
* @Date         : 2021-03-14
* @copyleft Apache 2.0
*/
#include "timeheap.h"

void TimeHeap::ShiftUp_(size_t i) {
    assert(i >= 0 && i < root_.size());
    // it start with 0 ,so it should be subject one.
    size_t j = (i - 1) / 2;
    while (j >= 0) {
        if (root_[j] < root_[i]) { break;}
        SwapNode_(i, j);
        i = j;
        j = (i - 1) / 2;
    }
}

void TimeHeap::SwapNode_(size_t i, size_t j) {
    assert(i >= 0 && i < root_.size());
    assert(j >= 0 && j < root_.size());
    std::swap(root_[i], root_[j]);
    ref_[root_[i].id] = i;
    ref_[root_[j].id] = j;
}

bool TimeHeap::ShiftDown_(size_t i, size_t n)  {
    assert(i >= 0 && i < root_.size());
    assert(n >= 0 && n <= root_.size());
    size_t index = i;
    size_t child = index * 2 + 1;
    while(child < n) {
        // find min value of child
        if(child + 1 < n && root_[child + 1] < root_[child]) child++;
        if(root_[index] < root_[child]) break;
        SwapNode_(index, child);
        index = child;
        child = child * 2 + 1;
    }
    return index > i;
}

void TimeHeap::Add(int id, int timeOut, const TimeoutCallBack &cb) {
    assert(id >= 0);
    size_t pos;
    if (ref_.count(id) == 0) {
        // add new one
        pos = root_.size();
        ref_[id] = pos;
        root_.push_back({id, Clock::now() + MS(timeOut), cb});
        ShiftUp_(pos);
    }
    else {
        // reset the time
        pos = ref_[id];
        root_[pos].expires = Clock::now() + MS(timeOut);
        root_[pos].cb = cb;
        if (!ShiftDown_(pos, root_.size())) {
            // not change
            ShiftUp_(pos);
        }
    }
}

void TimeHeap::DoWork(int id) {
    if (root_.empty() || ref_.count(id) == 0) { return ;}
    size_t pos = ref_[id];
    TimeNode tmp = root_[pos];
    tmp.cb();
    Delete_(pos);
}

void TimeHeap::Delete_(size_t i) {
    assert(!root_.empty() && i >= 0 && i < root_.size());
    size_t pos = i;
    size_t n = root_.size() - 1;
    assert(pos <= n);
    if (pos < n) {
        // move to the last value.
        SwapNode_(pos, n);
        if (!ShiftDown_(pos, n)) {
            ShiftUp_(pos);
        }
    }
    ref_.erase(root_.back().id);
    root_.pop_back();
}

void TimeHeap::Adjust(int id, int newExpires) {
    assert(!root_.empty() && ref_.count() > 0);
    root_[ref_[id]].expires = Clock::now() + MS(newExpires);
    ShiftDown_(ref_[id], root_.size());
}

void TimeHeap::Tick() {
    if (root_.empty()) return;
    while (!root_.empty()) {
        TimeNode node = root_.front();
        if (std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0) break;
        node.cb();
        assert(root_.empty());
        Delete_(0);
    }
}

void TimeHeap::Clear() {
    ref_.clear();
    root_.clear();
}

int TimeHeap::GetNextTick() {
    Tick();
    size_t res = -1;
    if (!root_.empty()) {
        res = std::chrono::duration_cast<MS>(root_.front().expires - Clock::now()).count();
        if (res < 0) res = 0;
    }
    return res;
}










