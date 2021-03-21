## MyWebServer
**基于C++14的高性能小型服务器**

### 项目亮点
* 封装Epoll库实现UnixIO模型中IO多路复用模型epoll
* 利用线程池实现多线程的Reactor和模拟Proactor高并发事件处理模型
* 使用正则表达式与状态机解析HTTP请求报文（支持解析**GET和POST**请求），并实现处理和回应静态资源请求
* 使用动态数组 + 互斥锁，信号量封装**自动增长**的缓冲区
* 使用小根堆封装定时器，实现自动关闭超时的非活动或者错误连接
* 利用单例模式（懒汉模式）与阻塞队列实现同步和异步的日志系统，记录服务器运行状态；
* 利用RAII机制实现了数据库连接池，减少数据库连接建立与关闭的开销，同时实现了用户注册登录功能。
* 经Webbench压力测试可以实现**上万的并发连接**数据交换


* 增加logsys,threadpool测试单元(todo: timer, sqlconnpool, httprequest, httpresponse) 

### 环境要求
* Linux 或 Mac os + docker 镜像
* C++14
* MySql

### 目录树
```

MyWebServer
├── LICENSE
├── code
│   ├── buffer
│   │   ├── buffer.cpp
│   │   ├── buffer.h
│   │   └── readme.md
│   ├── config
│   │   ├── Config.cpp
│   │   └── Config.h
│   ├── epoller
│   │   ├── Epoller.cpp
│   │   ├── Epoller.h
│   │   └── readme.md
│   ├── http
│   │   ├── httpconn.cpp
│   │   ├── httpconn.h
│   │   ├── httpreq.cpp
│   │   ├── httpreq.h
│   │   ├── httpresp.cpp
│   │   ├── httpresp.h
│   │   └── readme.md
│   ├── locker
│   │   ├── locker.h
│   │   └── readme.md
│   ├── log
│   │   ├── blockqueue.h
│   │   ├── log.cpp
│   │   ├── log.h
│   │   └── readme.md
│   ├── main.cpp
│   ├── pool
│   │   ├── readme.md
│   │   ├── sqlconnpool.cpp
│   │   ├── sqlconnpool.h
│   │   └── threadpool.h
│   ├── readme.md
│   ├── server
│   │   ├── readme.md
│   │   ├── webserver.cpp
│   │   └── webserver.h
│   └── timer
│       ├── readme.md
│       ├── timeheap.cpp
│       └── timeheap.h
├── readme.md
├── resource
│   ├── 400.html
│   ├── 403.html
│   ├── 404.html
│   ├── 405.html
│   ├── css
│   │   ├── animate.css
│   │   ├── bootstrap.min.css
│   │   ├── font-awesome.min.css
│   │   ├── magnific-popup.css
│   │   └── style.css
│   ├── error.html
│   ├── fonts
│   │   ├── FontAwesome.otf
│   │   ├── fontawesome-webfont.eot
│   │   ├── fontawesome-webfont.svg
│   │   ├── fontawesome-webfont.ttf
│   │   ├── fontawesome-webfont.woff
│   │   └── fontawesome-webfont.woff2
│   ├── images
│   │   ├── favicon.ico
│   │   ├── instagram-image1.jpg
│   │   ├── instagram-image2.jpg
│   │   ├── instagram-image3.jpg
│   │   ├── instagram-image4.jpg
│   │   ├── instagram-image5.jpg
│   │   └── profile-image.jpg
│   ├── index.html
│   ├── js
│   │   ├── bootstrap.min.js
│   │   ├── custom.js
│   │   ├── jquery.js
│   │   ├── jquery.magnific-popup.min.js
│   │   ├── magnific-popup-options.js
│   │   ├── smoothscroll.js
│   │   └── wow.min.js
│   ├── login.html
│   ├── picture.html
│   ├── register.html
│   ├── video
│   │   └── xxx.mp4
│   ├── video.html
│   └── welcome.html
├── test
│   ├── Makefile
│   ├── readme.md
│   └── test.cpp
└── webbench-1.5
    ├── Makefile
    ├── socket.c
    └── webbench.c
```


### 项目启动
需要先配置好对应的数据库
```bash
// 建立yourdb库
create database yourdb;

// 创建user表
USE yourdb;
CREATE TABLE user(
    username char(50) NULL,
    password char(50) NULL
)ENGINE=InnoDB;

// 添加数据
INSERT INTO user(username, password) VALUES('name', 'password');
```

```bash
make
./bin/server
```

### 压力测试
![image-webbench](https://github.com/markparticle/WebServer/blob/master/readme.assest/%E5%8E%8B%E5%8A%9B%E6%B5%8B%E8%AF%95.png)
```bash
./webbench-1.5/webbench -c 100 -t 10 http://ip:port/
./webbench-1.5/webbench -c 1000 -t 10 http://ip:port/
./webbench-1.5/webbench -c 5000 -t 10 http://ip:port/
./webbench-1.5/webbench -c 10000 -t 10 http://ip:port/
```
* 测试环境: Mac os + Docker cpu:i5-8400 内存:16G 
* QPS 10000+

### TODO
* 完善单元测试
* 实现循环缓冲区 最好使用循环队列
* 每一个部分的readme单独总结

### 致谢
Linux高性能服务器编程，游双著.

[@qinguoyi](https://github.com/qinguoyi/TinyWebServer)
