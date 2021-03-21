#include "config/Config.h"



int main(int argc, char *argv[])
{
    std::string username = "root";
    std::string password = "123456";
    std::string dbName = "wsx";

    Config config;
    config.ParseArg(argc, argv);

    int logQueueSize = config.LOGWrite == 0 ? 0 : 1024;

    WebServer server(
            config.PORT, config.TRIGMode, 60000, config.OPT_LINGER,                       /* 端口 ET模式 timeoutMs 优雅退出  */
            3306, username.c_str(), password.c_str(), dbName.c_str(),                                 /* Mysql配置 */
            config.ConnPoolNum, config.threadNum, config.OPENLOG, 1, (int)logQueueSize);    /* 连接池数量 线程池数量 日志开关 日志等级 日志异步队列容量 */
    server.Start();
    return 0;
}
