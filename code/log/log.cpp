/*
 * @Author       : wangshuxiao
 * @Date         : 2021-03-14
 * @copyleft Apache 2.0
 */
#include "log.h"

Log::log() {
    lineCount_ = 0;
    isAsync_ = false;
    writeThread_ = nullptr;
    deque_ = nullptr;
    today_ = 0;
    fp_ = nullptr;
}

Log::~Log() {
    if (writeThread_ && writeThread_->joinable())
}











