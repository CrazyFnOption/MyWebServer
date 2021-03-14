//
// Created by 王舒啸 on 2021/3/14.
//

#ifndef SQLCONNRAII_H
#define SQLCONNRAII_H

#include "sqlconnpool.h"

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

#endif //SQLCONNRAII_H
