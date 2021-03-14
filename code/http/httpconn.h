/*
 * @Author       : wangshuxiao
 * @Date         : 2021-03-14
 * @copyleft Apache 2.0
 */

#ifndef HTTPCONN_H
#define HTTPCONN_H

#include <sys/types.h>
#include <sys/uio.h>                                    // readv/writev
#include <arpa/inet.h>                                  // sockaddr_in
#include <stdlib.h>                                     // atoi()
#include <errno.h>

#include "../log/log.h"
#include "../pool/sqlconnRAII.h"
#include "../buffer/buffer.h"
#include "httpreq.h"
#include "httpresp.h"


class HttpConn {
    HttpConn();
    ~HttpConn();

    void init(int sockFd, const sockaddr_in& addr);
    void Close();

    ssize_t read(int* saveErrno);
    ssize_t write(int* saveErrno);

    int GetFd() const;
    int GetPort() const;
    const char* GetIP() const;
    sockaddr_in GetAddr() const;

    bool process();

    int ToWriteBytes() {
        return iov_[0].iov_len + iov_[1].iov_len;
    }

    bool IsKeepAlive() const {
        return request_.IsKeepAlive();
    }

    static bool isET;
    static const char* srcDir;
    static std::atomic<int> userCount;

private:

    int fd_;
    struct  sockaddr_in addr_;

    bool isClose_;

    int iovCnt_;
    struct iovec iov_[2];

    Buffer readBuff_; // 读缓冲区
    Buffer writeBuff_; // 写缓冲区

    HttpReq request_;
    HttpResp response_;
};


#endif //HTTPCONN_H
