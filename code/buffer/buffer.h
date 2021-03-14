/*
 * @Author       : wangshuxiao
 * @Date         : 2021-03-13
 * @copyleft Apache 2.0
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <iostream>
#include <unistd.h>
#include <sys/uio.h>
#include <string.h>
#include <string>
#include <vector>
#include <atomic>
#include <assert.h>


class Buffer {

public:
    Buffer(int initBuffSize = 1024);
    ~Buffer() = default;

    size_t WriteableBytes() const;
    size_t ReadableBytes() const;
    size_t PreBlankBytes() const;
    
    const char* Peek() const;
    void EusureWriteable(size_t len);
    void HasWritten(size_t len);

    void Take(size_t len);
    void TakeUntil(const char* end);
    void TakeAll();
    std::string TakeAllToStr();

    const char* BeginWriteConst() const;
    char* BeginWrite();

    void Append(const std::string& str);
    void Append(const char* str, size_t len);
    void Append(const void* data, size_t len);          // This will be transformed to any type
    void Append(const Buffer& buff);

    ssize_t ReadFd(int fd, int* Errno);                 // read from file to buffer
    ssize_t WriteFd(int fd, int* Errno);                // write from buffer to file

private:
    char* Begin_();
    const char* Begin_() const;
    void ArrangeSpace_(size_t len);

    std::vector<char> buffer_;
    std::atomic<std::size_t> readpos_;
    std::atomic<std::size_t> writepos_;
};

#endif //BUFFER_H