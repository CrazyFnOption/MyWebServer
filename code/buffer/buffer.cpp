/*
 * @Author       : wangshuxiao
 * @Date         : 2021-03-13
 * @copyleft Apache 2.0
 */

#include "buffer.h"

Buffer::Buffer(int initBuffSize): buffer_(initBuffSize), readpos_(0), writepos_(0) {}

size_t Buffer::ReadableBytes() const {
    return writepos_ - readpos_;
}

size_t Buffer::WriteableBytes() const {
    return buffer_.size() - writepos_;
}

size_t Buffer::PreBlankBytes() const {
    return readpos_;
}

const char* Buffer::Peek() const {
    return Begin_() + readpos_;
}

void Buffer::Take(size_t len) {
    assert(len <= ReadableBytes());
    readpos_ += len;
}

void Buffer::TakeUntil(const char *end) {
    assert(Peek() <= end);
    take(end - Peek());
}

void Buffer::TakeAll(){
    bzero(&buffer_[0], buffer_.size());
    writepos_ = readpos_ = 0;
}

std::string Buffer::TakeAllToStr() {
    std::string str(Peek(), ReadableBytes());
    TakeAll();
    return str;
}

const char* Buffer::BeginWriteConst() const {
    return Begin_() + writePos_;
}

char* Buffer::BeginWrite() {
    return Begin_() + writePos_;
}

void Buffer::HasWritten(size_t len) {
    writePos_ += len;
}

void Buffer::Append(const std::string& str) {
    Append(str.data(), str.length());
}

void Buffer::Append(const void* data, size_t len) {
    assert(data);
    Append(static_cast<const char*>(data), len);
}

void Buffer::Append(const char *str, size_t len) {
    assert(str);
    EusureWriteable(len);
    std::copy(str, str + len, BeginWrite());
    HasWritten(len);
}

void Buffer::Append(const Buffer &buff) {
    assert(buff);
    Append(buff.Peek(), buff.ReadableBytes());
}

void Buffer::EusureWriteable(size_t len) {
    if(WriteableBytes() < len) {
        ArrangeSpace_(len);
    }
    assert(WriteableBytes() >= len);
}

ssize_t Buffer::ReadFd(int fd, int *Errno) {
    char buffer[65535];
    struct iovec iov[2];
    const size_t writeable = WriteableBytes();
    // if the space is not enough , it will be send to buffer
    iov[0].iov_base = Begin_() + writepos_;
    iov[0].iov_len = writeable;
    iov[1].iov_base = buffer;
    iov[1].iov_len = sizeof(buffer);

    const ssize_t len = readv(fd, iov, 2);
    if (len < 0) {
        *Errno = errno;
    }
    else if (static_cast<size_t>(len) <= writeable) {
        writepos_ += len;
    }
    else {
        writepos_ = buffer_.size();
        Append(buffer, len - writeable);
    }
    return len;
}

ssize_t Buffer::WriteFd(int fd, int *Errno) {
    size_t readSize = ReadableBytes();
    ssize_t len = write(fd, Peek(), readSize);
    if (len < 0) {
        *Errno = errno;
        return len;
    }
    readpos_ += len;
    return len;
}

char* Buffer::Begin_() {
    return &(*buffer_.begin());
}

const char* Buffer::Begin_() const{
    return &(*buffer_.begin());
}

void Buffer::ArrangeSpace_(size_t len) {
    if (WriteableBytes() + PreBlankBytes() < len) {
        buffer_.resize(WriteableBytes() + len + 1);
    }
    else {
        size_t readSize = ReadableBytes();
        std::copy(Begin_() + readpos_, Begin_() + writepos_, Begin_());
        readpos_ = 0;
        writepos_ = readpos_ + readSize;
        assert(readable == ReadableBytes());
    }
}








