//
// Created by 王舒啸 on 2021/3/21.
//

#include "Config.h"
#include <stdlib.h>

Config::Config() {
    PORT = 9006;                                        // port
    LOGWrite = 0;                                       // 0 for Sync , 1 for ASync
    TRIGMode = 0;                                       // default for LT + LT
    OPT_LINGER = 0;                                     // default for not use classic close
    ConnPoolNum = 12;                                    // default sql connection for 8
    threadNum = 6;                                      // the num of thread pool is 8;
    OPENLOG = 1;                                        // 1 for open , 0 for close.
    ActorModel = 0;                                     // 0 for proactor , 1 for reactor
}

void Config::ParseArg(int argc, char **argv) {
    int opt;
    const char * str = "p:l:m:o:s:t:c:a:";
    while ((opt = getopt(argc, argv, str)) != -1) {
        switch (opt)
        {
            case 'p':
                PORT = atoi(optarg);
                break;
            case 'l':
                LOGWrite = atoi(optarg);
                break;
            case 'm':
                TRIGMode = atoi(optarg);
                break;
            case 'o':
                OPT_LINGER = atoi(optarg);
                break;
            case 's':
                ConnPoolNum = atoi(optarg);
                break;
            case 't':
                threadNum = atoi(optarg);
                break;
            case 'c':
                OPENLOG = atoi(optarg);
                break;
            case 'a':
                ActorModel = atoi(optarg);
                break;
            default: break;
        }
    }
}