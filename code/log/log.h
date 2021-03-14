///*
// * @Author       : wangshuxiao
// * @Date         : 2021-03-14
// * @copyleft Apache 2.0
// */

#ifndef LOG_H
#define LOG_H

#include <mutex>
#include <string>
#include <thread>
#include <sys/time.h>
#include <string.h>
#include <stdarg.h>                                     // vastart va_end
#include <assert.h>
#include <sys/stat.h>

#include "blockqueue.h"
#include "../buffer/buffer.h"


class Log {
public:
    void init(int level, const char* path="./log",
              const char* suffix = ".log",
              int maxCapacity = 1024);

    static Log* GetInstance();
    static void FlushLogThread();

    void Write(int level, const char* format,...);
    void Flush();

    int GetLevel();
    void SetLevel(int level);
    bool IsOpen() {return isOpen_;}

private:
    Log();
    void AppendLogLevelTitle_(int level);
    virtual ~Log();
    void AsyncWrite_();

private:
    static const int LOG_PATH_LEN = 256;
    static const int LOG_NAME_LEN = 256;
    static const int MAX_LINES = 50000;

    const char* path_;
    const char* suffix_;

    int maxLines_;
    int lineCount_;
    int today_;

    bool isOpen_;

    Buffer buffer_;
    int level_;                             // log type
    bool isAsync_;                          // false for Sync true for Async

    FILE* fp_;
    std::unique_ptr<BlockQueue<std::string> > deque_;
    std::unique_ptr<std::thread>writeThread_;
    std::mutex mtx_;
};

#define LOG_BASE(level, format, ...) \
    do {\
        Log* log = Log::Instance();\
        if (log->IsOpen() && log->GetLevel() <= level) {\
            log->write(level, format, ##__VA_ARGS__); \
            log->flush();\
        }\
    } while(0);

#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);

#endif //LOG_H
