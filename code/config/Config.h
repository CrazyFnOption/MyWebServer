//
// Created by 王舒啸 on 2021/3/21.
//

#ifndef CONFIG_H
#define CONFIG_H

#include <unistd.h>

class Config {
public:
    Config();
    ~Config() = default;

    void ParseArg(int argc, char* argv[]);

public:
    int PORT;
    int TRIGMode;
    int LOGWrite;
    int ConnPoolNum;
    int threadNum;
    int OPENLOG;
    int OPT_LINGER;
    int ActorModel;

};


#endif //CONFIG_H
