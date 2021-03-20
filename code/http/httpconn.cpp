/*
* @Author       : wangshuxiao
* @Date         : 2021-03-14
* @copyleft Apache 2.0
*/

#include "httpconn.h"

using namespace std;

const char* HttpConn::srcDir;
std::atomic<int> HttpConn::userCount = 0;
bool HttpConn::isET = false;

HttpConn::HttpConn() {
    fd_ = -1;
    addr_ = {0};
    isClose_ = true;
}

HttpConn::~HttpConn() {
    Close();
}

void HttpConn::Init(int sockFd, const sockaddr_in &addr) {
    assert(sockFd > 0);
    userCount++;
    addr_ = addr;
    fd_ = sockFd;
    writeBuff_.TakeAll();
    readBuff_.TakeAll();
    isClose_ = false;
    LOG_INFO("Client[%d](%s:%d) in, userCount:%d", fd_, GetIP(), GetPort(), (int)userCount);
}

void HttpConn::Close() {
    response_.UnmapFile();
    if (!isClose_) {
        isClose_ = true;
        userCount--;
        close(fd_);
        LOG_INFO("Client[%d](%s:%d) quit, UserCount:%d", fd_, GetIP(), GetPort(), (int)userCount);
    }
}

int HttpConn::GetFd() const {
    return fd_;
};

struct sockaddr_in HttpConn::GetAddr() const {
    return addr_;
}

const char* HttpConn::GetIP() const {
    // inet_ntoa   transform IPV4 into Internet standard dotted format
    return inet_ntoa(addr_.sin_addr);
}

int HttpConn::GetPort() const {
    return addr_.sin_port;
}

ssize_t HttpConn::Read(int *saveErrno) {
    ssize_t len = -1;
    do {
        len = readBuff_.ReadFd(fd_, saveErrno);
        if (len <= 0) break;
    }while(isET);
    return len;
}

ssize_t HttpConn::Write(int *saveErrno) {
    ssize_t len = -1;
    do {
        len = writev(fd_, iov_, iovCnt_);
        if (len <= 0) {
            *saveErrno = errno;
            break;
        }
        if(iov_[0].iov_len + iov_[1].iov_len  == 0) break;
        else if (static_cast<size_t>(len) > iov_[0].iov_len) {
            iov_[1].iov_base = (uint8_t*) iov_[1].iov_base + (len - iov_[0].iov_len);
            iov_[1].iov_len -= (len - iov_[0].iov_len);
            if (iov_[0].iov_len) {
                writeBuff_.TakeAll();
                iov_[0].iov_len = 0;
            }
        }
        else {
            iov_[0].iov_base = (uint8_t*)iov_[0].iov_base + len;
            iov_[0].iov_len -= len;
            writeBuff_.Take(len);
        }
    }while(isET || ToWriteBytes() > 10240);
}

bool HttpConn::Process() {
    request_.Init();
    if (readBuff_.ReadableBytes() <= 0) return false;
    else if (request_.Parse(readBuff_)) {
        LOG_DEBUG("%s", request_.path().c_str());
        response_.Init(srcDir, request_.path(),
                       request_.IsKeepAlive(), 200);
    }
    else response_.Init(srcDir, request_.path(), false, 400);

    response_.MakeResponse(writeBuff_);

    // response header
    iov_[0].iov_base = const_cast<char*>(writeBuff_.Peek());
    iov_[0].iov_len = writeBuff_.ReadableBytes();
    iovCnt_ = 1;

    // response file
    if (response_.FileLen() > 0 && response_.File()) {
        iov_[1].iov_base = response_.File();
        iov_[1].iov_len = response_.FileLen();
        iovCnt_ = 2;
    }
    LOG_DEBUG("filesize:%d, %d  to %d", response_.FileLen() , iovCnt_, ToWriteBytes());
    return true;
}






