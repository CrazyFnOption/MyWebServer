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
    if (writeThread_ && writeThread_->joinable()) {
        while (!deque_->Empty()) {
            // still have some work to do
            deque_->Flush();
        }
        deque_->Close();
        writeThread_->join();
    }
    if(fp_) {
        std::lock_guard<std::mutex>locker (mtx_);
        Flush();
        fclose(fp_);
    }
}

int Log::GetLevel() {
    std::lock_guard<std::mutex>locker (mtx_);
    return level_;
}

void Log::SetLevel(int level) {
    std::lock_guard<std::mutex>locker (mtx_);
    level_ = level;
}

void Log::init(int level, const char *path, const char *suffix, int maxCapacity) {
    isOpen_ = true;
    level_ = level;
    // use the asynchronous way.
    if (maxCapacity > 0) {
        isAsync_ = true;
        if (!deque_) {
            std::unique_ptr<BlockQueue<std::string> > newDeque(new BlockQueue<std::string>);
            deque_ = std::move(newDeque);

            std::unique_ptr<std::thread> NewThread(new thread(FlushLogThread));
            writeThread_ = std::move(NewThread);
        }
    }
    else {
        // use the sync's way
        isAsync_ = false;
    }

    // set time
    lineCount_ = 0;
    time_t timer = time(nullptr);
    struct tm* sysTime = localtime(&timer);
    struct tm t = *sysTime;
    path_ = path;
    suffix_ = suffix;
    char fileName[LOG_NAME_LEN] = {0};
    snprintf(fileName, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d%s",
             path_, t.tm_year, t.tm_mon + 1, t.tm_mday, suffix_);
    today_ = t.tm_mday;

    // maybe exist last not close file
    {
        std::lock_guard<std::mutex>locker (mtx_);
        // don't care whether if still has bytes
        buffer_.TakeAll();
        if(fp_) {
            Flush();
            fclose(fp_);
        }

        // open the new file
        fp_ = fopen(fileName, "a");
        if (fp == nullptr) {
            mkdir(path_, 0777)
            fp_ = fopen(fileName, "a");
        }
        assert(fp_ != nullptr);
    }
}

void Log::AppendLogLevelTitle_(int level) {
    switch(level) {
        case 0:
            buffer_.Append("[debug]: ", 9);
            break;
        case 1:
            buffer_.Append("[info] : ", 9);
            break;
        case 2:
            buffer_.Append("[warn] : ", 9);
            break;
        case 3:
            buffer_.Append("[error]: ", 9);
            break;
        default:
            buffer_.Append("[info] : ", 9);
            break;
    }
}

void Log::GetInstance() {
    static Log instance;
    return &instance;
}

void Log::AsyncWrite_() {
    std::string str = "";
    while(deque_->Pop(str)) {
        std::lock_guard<std::mutex>locker (mtx_);
        fputs(str.c_str(), fp_);
    }
}

void Log::FlushLogThread() {
    Log::GetInstance()->AsyncWrite_();
}

void Log::Flush() {
    if (isAsync_) {
        deque_->Flush();
    }
    fflush(fp_);
}

void Log::Write(int level, const char *format, ...) {
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);
    time_t tSec = now.tv_sec;
    struct tm *sysTime = localtime(&tSec);
    struct tm t = *sysTime;

    va_list vaList;

    // the wrong day or no space to write
    // so have to change new file
    if (toDay_ != t.tm_mday || (lineCount_ && (lineCount_  %  MAX_LINES == 0)))
    {
        std::unique_lock<std::mutex> locker(mtx_);
        locker.unlock();

        char newFile[LOG_NAME_LEN];
        char tail[36] = {0};
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

        if (toDay_ != t.tm_mday)
        {
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s%s", path_, tail, suffix_);
            toDay_ = t.tm_mday;
            lineCount_ = 0;
        }
        else {
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s-%d%s", path_, tail, (lineCount_  / MAX_LINES), suffix_);
        }

        locker.lock();
        Flush();
        fclose(fp_);
        fp_ = fopen(newFile, "a");
        assert(fp_ != nullptr);
    }

    {
        std::unique_lock<std::mutex> locker(mtx_);
        lineCount_++;
        int n = snprintf(buffer_.BeginWrite(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
                         t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                         t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);

        buffer_.HasWritten(n);
        AppendLogLevelTitle_(level);

        // write into the buffer
        va_start(vaList, format);
        int m = vsnprintf(buffer_.BeginWrite(), buffer_.WriteableBytes(), format, vaList);
        va_end(vaList);

        buffer_.HasWritten(m);
        buffer_.Append("\n\0", 2);

        // write into the file
        // Async's way to write
        if(isAsync_ && deque_ && !deque_->full()) {
            deque_->push_back(buffer_.TakeAllToStr());
        } else {
            // Sync's way to write
            fputs(buffer_.Peek(), fp_);
        }
        buffer_.TakeAll();
    }
}








