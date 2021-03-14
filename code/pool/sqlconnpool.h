/*
 * @Author       : wangshuxiao
 * @Date         : 2021-03-13
 * @copyleft Apache 2.0
 */


#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <thread>
#include "../log/log.h"

class SqlConnPool {
public:
    static SqlConnPool* GetInstence();
    MYSQL* GetConn();
    void FreeConn(MYSQL* conn);
    int GetFreeConnCount();

    void Init(const char* host, int port, const char* user,
              const char* pwd, const char* dbName, int connSize);
    void ClosePool();

private:
    SqlConnPool();
    ~SqlConnPool();

    int MAX_CONN_;
    int useCount_;
    int freeCount_;

    std::queue<MYSQL* > connQue_;
    std::mutex mtx_;
    sem_t semId_;

};

// Resource initializationin when Object Construction
// Destroy when Object DeConstruction
class SqlConnRAII {
public:
    SqlConnRAII(MYSQL **sql, SqlConnPool* connPool) {
        assert(connPool);
        *sql = connPool->GetConn();
        sql_ = *sql;
        connPool_ = connPool;
    }

    ~SqlConnRAII() {
        if (sql_) {
            connPool_->FreeConn(sql_);
        }
    }

private:
    MYSQL* sql_;
    SqlConnPool* connPool_;
};

#endif //SQLCONNPOOL_H
