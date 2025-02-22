# 负载均衡式的在线OJ

## 1所用技术与开发环境

所用技术：

- c++ stl标准库
- Boost准标准库(字符串切割)
- cpp-httplib第三方开源网络库
- ctemplate第三方开源前端网页渲染库
- jsoncpp第三方开源序列化，反序列化库
- 负载均衡设计
- 多进程、多线程
- MySQL C connect
- Ace前端在线编辑器
- html/css/js/jquery/ajax

开发环境：

- Unbunto云服务器
- vscode

## 2 项目宏观结构

实现类似leetcode的题目列表+在线编程功能

项目核心是3个模块：

1 comm:公共模块

2 compile_serve r:编译与运行模块

3 oj_server: 获取题目列表，查看题目编写题目界面，负载均衡，其他功能

![image-20250220150017574](assets/image-20250220150017574.png)

服务器会收到各种请求：

1 如果收到 请求题目列表或特定题目编写等请求，不需要对代码进行编译/提交，通过oj_server去访问后端数据库或文件，拉取对应的题目，返回给客户端；

2 如果用户要提交代码，oj_server就要负载均衡式的选择某一台后端主机进行编译，以网络形式提供编译服务，编译完成后把结果交给oj_server，由oj_server返回给客户端。

## 3 编写思路

1 先编写 compile_server

2 oj_server

3 版本1基于文件版的在线oj

4 前端的页面设计

5 版本2基于MySQL版的在线oj

## 4 compiler服务设计

提供的服务：编译并运行代码，得到格式化的相关结果

![image-20250220180732596](assets/image-20250220180732596.png)

### 编译功能

```c++
    //引入路径拼接功能
    using namespace ns_util;
    using namespace ns_log;

    //返回值：编译成功，true;否则false
    //file_name:1234(传入只有文件名，没有后缀，手动拼接),因为在编译中会形成各种临时文件
    //1234 -> ./temp/1234.cpp
    //1234 -> ./temp/1234.exe
    //1234 -> ./temp/1234.stderr 编译错误形成的文件
    static bool Compile(const std::string &file_name)
    {
         //首先创建子进程进行编译
        pid_t id = fork();
        if(id == 0)//子进程
        {
            int _stderr = open(PathUtil::Stderr(file_name).c_str(),O_CREAT|O_WRONLY,0644); 
            if(_stderr < 0)
            {
                LOG(WARNING) << "没有成功形成strerr文件"<<std::endl;
                exit(1);
            }
            //标准错误重定向到_stderr
            dup2(_stderr,2);
            //程序替换不会影响进程的文件描述符表

            //调用编译器，完成对代码的编译工作
            //g++ -o target src -std=c++11
            execlp("g++","g++","-o",PathUtil::Exe(file_name).c_str(),\
            PathUtil::Src(file_name).c_str(),"-std=c++11",nullptr);
            LOG(ERROR) << "启动编译器g++失败，参数错误？"<<std::endl;
            //程序替换失败，直接终止子进程
            exit(2);
        }
        else if(id > 0) //父进程
        {
            waitpid(id,nullptr,0);
            //编译是否成功
            //就看对应的.exe文件是否存在即可
            if(FileUtil::IsFileExists(PathUtil::Exe(file_name)))
            {
                LOG(INFO) << PathUtil::Src(file_name)<<"\n";
                LOG(INFO) << "编译成功" << "\n";
                return true;
            }
            LOG(INFO) << PathUtil::Src(file_name)<<"\n";
            LOG(INFO) << "编译失败,没有形成可执行程序"<<"\n";
            return false;
        }
        else//创建子进程失败
        {
            LOG(ERROR) << "编译模块创建子进程失败"<<std::endl;
            return false;
        }
    }
```

### 测试资源限制

```
#include <iostream>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <signal.h>
//没有做相关的资源约束，一直在死循环
// int main()
// {
//     while(1);
//     return 0;
// }

void handler(int signal)
{
    std::cout << "收到信号："<<signal << "\n";
    exit(1);
}
int main()
{
    //资源不足，导致os终止进程，是通过信号终止的 
    for(int i = 1; i <= 31; ++i)
    {
        signal(i,handler);
    }

    // //限制累计运行时长
    // struct rlimit r;
    // r.rlim_cur = 1;//最多1秒
    // r.rlim_max = RLIM_INFINITY;//硬上限设置为无穷大
    // setrlimit(RLIMIT_CPU,&r);//设置资源约束
    // while(1);
    
    //限制空间大小
    struct rlimit r;
    //注意：程序本身加载也会占用空间
    r.rlim_cur = 1024*1024*40;//40M
    r.rlim_max = RLIM_INFINITY;
    setrlimit(RLIMIT_AS,&r);
    int count = 0;
    while(true)
    {
        int* p = new int[1024*1024];//1M
        count++;
        std::cout << "申请空间：" << count << " M" << std::endl;
        sleep(1);
    }
    return 0;
}

//内存申请失败
terminate called after throwing an instance of 'std::bad_alloc'
  what():  std::bad_alloc
收到信号：6
kill -l
 1) SIGHUP       2) SIGINT       3) SIGQUIT      4) SIGILL       5) SIGTRAP
 6) SIGABRT      7) SIGBUS       8) SIGFPE       9) SIGKILL     10) SIGUSR1
11) SIGSEGV     12) SIGUSR2     13) SIGPIPE     14) SIGALRM     15) SIGTERM
16) SIGSTKFLT   17) SIGCHLD     18) SIGCONT     19) SIGSTOP     20) SIGTSTP
21) SIGTTIN     22) SIGTTOU     23) SIGURG      24) SIGXCPU     25) SIGXFSZ
26) SIGVTALRM   27) SIGPROF     28) SIGWINCH    29) SIGIO       30) SIGPWR
31) SIGSYS

//CPU使用超时
./a.out
收到信号：24

//设置进程占用资源大小的接口
static void SetProcLimit(int cpu_limit,int mem_limit)
{
    //设置CPU时长
    struct rlimit cpu_rlimit;
    cpu_rlimit.rlim_cur = cpu_limit;
    cpu_rlimit.rlim_max = RLIM_INFINITY;
    setrlimit(RLIMIT_CPU,&cpu_rlimit);

    //设置内存大小  mem_limit KB
    struct rlimit mem_rlimit;
    mem_rlimit.rlim_cur = mem_limit * 1024;//它本身是以byte为单位，所以要*1024转换成KB
    mem_rlimit.rlim_max = RLIM_INFINITY;
    setrlimit(RLIMIT_AS,&mem_rlimit);
}
```

### 运行功能

```
        //指名文件名，不需要带路径和后缀
        //0：表示程序正常运行完成，没有发生异常
        //>0：表示程序运行时收到了信号，程序异常,
        //<0:内部错误
        //cpu_limit:该程序运行时，可以使用的最大cpu资源上限
        //mem_limit:该程序运行时，可以使用的最大内存大小(KB)
        static int Run(const std::string&file_name, int cpu_limit,int mem_limit)
        {
            /*
            程序运行：
            1 代码跑完，结果 正确/不正确
            2 代码没跑完，中途收到信号退出
            */
           /*
                Run只需要考虑是否运行完毕
                结果正确与否，是由测试用例决定
           */

            /*
            一个程序在默认启动时：
            标准输入：不处理
            标准输出：程序运行完成，输出的内容
            标准错误：运行时错误信息
            */
            umask(0);
            int stdin_fd = open(PathUtil::Stdin(file_name).c_str(),O_CREAT|O_RDONLY,0644);
            int stdout_fd = open(PathUtil::Stdout(file_name).c_str(),O_CREAT|O_WRONLY,0644);
            int stderr_fd = open(PathUtil::Stderr(file_name).c_str(),O_CREAT|O_WRONLY,0644);
            if(stdin_fd < 0 || stdout_fd < 0 || stderr_fd < 0)
            {
                LOG(ERROR) << "运行时，打开标准文件失败" << std::endl;
                return -1;
            }
            pid_t id = fork();
            if(id < 0)
            {
                close(stdin_fd);
                close(stdout_fd);
                close(stderr_fd);
                LOG(ERROR) << "运行时创建子进程失败" << std::endl;
                return -2;
            }
            else if(id == 0)//子进程
            {
                //int setrlimit(int resource,const struct rlimit* rlim)设置一个进程占用资源的上限
                //第一个参数
                //RLIMIT_AS:占的虚拟内存大小
                //RLIMIT_CPU:占用的cpu时间
                //第二个参数
                /*
                    struct rlimit{
                        rlim_t rlim_cur;//约束占用资源的上限
                        rlim_t rlim_max;//设置rlim_cur的最大值,默认无穷大
                    };
                */
                //给子进程添加资源约束，子进程的资源使用情况将以信号方式向父进程呈现,资源越界就会返回特定信号
                //把程序运行的标准输入、输出、错误重定向到临时文件中
                dup2(stdin_fd,0);
                dup2(stdout_fd,1);
                dup2(stderr_fd,2);
                //设置进程资源限制(运行时长和空间占用)，内部封装setrlimit
                SetProcLimit(cpu_limit,mem_limit);
                execl(PathUtil::Exe(file_name).c_str(),PathUtil::Exe(file_name).c_str(),nullptr);
                exit(1);
            }
            else if(id > 0)//父进程
            {
                close(stdin_fd);
                close(stdout_fd);
                close(stderr_fd);
                int ret = 0;
                waitpid(id,&ret,0);
                //最后面7位代表收到的信号，0表示没有收到信号
                LOG(INFO) << "运行完毕，info:" << (ret & 0x7f) << std::endl;
                //通过判断信号，可以得知因为什么原因出错
                return ret & 0x7f;
            }
        }
```

### 编译并运行功能

#### 当前模块软件结构

![image-20250222111159171](assets/image-20250222111159171.png)

#### 认识jsoncpp

```c++
// 2 g++ test.cc -std=c++11 -ljsoncpp 要链接json库
//apt-get install libjsoncpp-dev
int main()
{
    
    //序列化工作
    //将结构化的数据，转换成一个字符串
    //Value是一个Json的中间类，可以填充KV值
    Json::Value root;
    root["code"] = "mycode"; 
    root["user"] = "李勇豪";
    root["age"] = "18";

    //用来进行序列化工作的类对象
    //Json::StyledWriter writer;
    //Json::FastWriter writer; 中文会乱码
    Json::StreamWriterBuilder writer;
    writer.settings_["emitUTF8"] = true;
    std::string str = Json::writeString(writer,root);
    std::cout << str << "\n";
    return 0;
}

{
        "age" : "18",
        "code" : "mycode",
        "user" : "李勇豪"
}
```

#### 编译并运行

```c++
//1 适配用户请求,定制通信协议字段
//2 正确调用compile and run
//3 形成唯一文件名 —— 编译服务随时可能被多个人请求，必须保证传递上来的code，形成源文件名称的时候，要具有唯一性，不然多个用户之间，会互相影响
#pragma once
#include "compiler.hpp"
#include "runner.hpp"
#include <jsoncpp/json/json.h>
#include "../comm/log.hpp"
#include "../comm/util.hpp"
//整合，在内部即调用compile.hpp 也调用 runner.hpp
namespace ns_compile_and_run
{
    using namespace ns_log;
    using namespace ns_util;
    using namespace ns_compiler;
    using namespace ns_runner;
    class CompileAndRun
    {
    public:
        //code > 0 进程收到信号导致进程退出/运行时报错
        //code < 0 非运行的报错(代码为空/编译报错/内部错误)
        //code = 0 成功
        static std::string CodeDesc(int code,const std::string& file_name)
        {
            std::string desc;
            switch(code)
            {
            case 0:
                desc = "编译运行成功";
                break;
            case -1:
                desc = "代码为空";
                break;
            case -2:
                desc = "未知错误/内部错误";
                break;
            case -3:
                //desc = "编译发生错误";
                FileUtil::ReadFile(PathUtil::CompilerError(file_name),&desc,true);
                break;
            case SIGABRT://6
                desc = "空间复杂度过高";
                break;
            case SIGXCPU://24
                desc = "时间复杂度过高";
                break;
            case SIGFPE://8
                desc = "浮点数溢出/除0错误";
                break;
            default:
                desc = "收到信号：" + std::to_string(code);
                break;
            }
            return desc;
        }
        /*
        json字符串里的key
        输入：
        code: 用户提交的代码
        input:用户给自己提交的代码对应的输入,方便扩展
        cpu_limit:时间要求   单位s
        mem_limit:空间要求   单位KB
        
        输出：
        必填字段
        status：状态码 0表示成功 <0发生错误
        reason：请求结果
        选填字段
        stdout：用户代码运行完的结果
        stderr：用户代码运行完的错误结果

        参数：
        in_json: {"code":"","input":"","cpu_limit":1,"mem_limit":1024}
        
        out_json:{"status":"0","reason":"运行未发生错误","stdout":"...","stderr":"..."}

        */
        static void Start(const std::string& in_json, std::string *out_json)
        {
            //反序列化：将1个字符串解析成多个kv值
            Json::Value in_value;
            Json::Reader reader;
            reader.parse(in_json,in_value);

            std::string code = in_value["code"].asString();
            std::string input = in_value["input"].asString();
            int cpu_limit = in_value["cpu_limit"].asInt();
            int mem_limit = in_value["mem_limit"].asInt();

            int status_code = 0;
            Json::Value out_value;
            std::string file_name;//形成的唯一文件名
            int run_code = 0; //代码运行的返回值

            //在goto 和 END 之间不能存在定义
            if(code.size() == 0)
            {
                status_code = -1;//代码为空
                goto END;
            }
            file_name = FileUtil::UniqFileName();
            if(!FileUtil::WriteFile(PathUtil::Src(file_name),code))
            {
                status_code = -2;//未知错误/内部错误
                goto END;
            } 

            if(!Compiler::Compile(file_name))
            {
                status_code = -3;//编译失败
                goto END;
            }
            run_code = Runner::Run(file_name,cpu_limit,mem_limit);
            if(run_code < 0)
            {
                status_code = -2;//未知错误/内部错误
            }
            else if(run_code > 0)
            {
                //程序运行崩溃，收到信号
                status_code = run_code;
            }
            else
            {
                //运行成功
                status_code = 0;
            }
        END:
            out_value["status"] = status_code;
            out_value["reason"] = CodeDesc(status_code,file_name);
            if(status_code == 0)
            {
                std::string std_out;
                std::string std_err;
                FileUtil::ReadFile(PathUtil::Stdout(file_name),&std_out,true);
                FileUtil::ReadFile(PathUtil::Stderr(file_name),&std_err,true);
                //整个过程全部成功，才需要填充stdout
                out_value["stdout"] = std_out;
                out_value["stderr"] = std_err;
            }
            //反序列化
            Json::StreamWriterBuilder writer;
            writer.settings_["emitUTF8"] = true;
            *out_json = Json::writeString(writer,out_value);
            //把结果全部拿到以后，清理所有相关临时文件
			FileUtil::RemoveTempFile(file_name);
        }
    };
```

测试：

```
int main()
{
    // testCompile("code");
    // testExecute("code");

    //通过http 让client给我们上传一个json string数据
    std::string in_json;
    Json::Value in_value;
    //in_value["code"] = "#include<iostream> \n int main(){\n std::cout<<\"你好\"<<std::endl \n return 0;}";
    //R"()",raw string,在括号里的字符串，如果出现特殊字符，保持原貌
    in_value["code"] = R"(#include<iostream>
    int main()
    {
        std::cout<<"你好"<<std::endl; 
        aaaaaa
        return 0;
    })";
    in_value["input"] = "";
    in_value["cpu_limit"] = 1;
    in_value["mem_limit"] = 1024 * 100;//单位是KB，所以空间大小为100M
    
    Json::StreamWriterBuilder writer;
    writer.settings_["emitUTF8"] = true;

    in_json = Json::writeString(writer,in_value);
    

    std::string out_json;//将来给客户端浏览器返回的字符串
    CompileAndRun::Start(in_json,&out_json);

    std::cout << out_json << std::endl;
    return 0;
}

[INFO][compiler.hpp][61][1740224530]./temp/1740224530359xx0.cpp
[INFO][compiler.hpp][62][1740224530]编译失败,没有形成可执行程序
{
        "reason" : "./temp/1740224530359xx0.cpp: In function ‘int main()’:\n./temp/1740224530359xx0.cpp:5:9: error: ‘aaaaaa’ was not declared in this scope\n    5 |         aaaaaa\n      |         ^~~~~~\n",
        "status" : -3
}

[INFO][compiler.hpp][58][1740224285]./temp/1740224285023xx0.cpp编译成功
[INFO][runner.hpp][92][1740224285]运行完毕，info:8
{
        "reason" : "浮点数溢出/除0错误",
        "status" : 8
}

[INFO][compiler.hpp][58][1740224236]./temp/1740224236170xx0.cpp编译成功
[INFO][runner.hpp][92][1740224236]运行完毕，info:6
{
        "reason" : "空间复杂度过高",
        "status" : 6
}

[INFO][compiler.hpp][58][1740224003]./temp/1740224003243xx0.cpp编译成功
[INFO][runner.hpp][92][1740224004]运行完毕，info:24
{
        "reason" : "时间复杂度过高",
        "status" : 24
}

[INFO][compiler.hpp][58][1740223583]./temp/1740223582975xx0.cpp编译成功
[INFO][runner.hpp][92][1740223583]运行完毕，info:0
{
        "reason" : "编译运行成功",
        "status" : 0,
        "stderr" : "",
        "stdout" : "你好\n"
}
```

#### 清理临时文件

```
//把结果全部拿到以后，清理所有相关临时文件
FileUtil::RemoveTempFile(file_name);

//删除所有与file_name相关的临时文件
static void RemoveTempFile(const std::string& file_name)
{
    //清理文件的个数不确定
    //逐个判断
    std::string src = PathUtil::Src(file_name);
    if(FileUtil::IsFileExists(src)) 
        unlink(src.c_str());

    std::string compiler_error = PathUtil::CompilerError(file_name);
    if(FileUtil::IsFileExists(compiler_error)) 
        unlink(compiler_error.c_str());

    std::string execute = PathUtil::Exe(file_name);
    if(FileUtil::IsFileExists(execute))
        unlink(execute.c_str());

    std::string _stdin = PathUtil::Stdin(file_name);
    if(FileUtil::IsFileExists(_stdin))
        unlink(_stdin.c_str());

    std::string _stdout = PathUtil::Stdout(file_name);
    if(FileUtil::IsFileExists(_stdout))
        unlink(_stdout.c_str());

    std::string _stderr = PathUtil::Stderr(file_name);
    if(FileUtil::IsFileExists(_stderr))
        unlink(_stderr.c_str());
}
```

#### 打包成网络服务

cpp-httplib使用实例：

```c++
//接入cpp-httplib:header-only 只需要将.h拷贝到项目中，即可直接使用
//cpp-httplib:需要使用高版本的gcc/g++
//gcc version 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04) 
//cpp-httplib:阻塞式多线程的一个网络http库,需要链接lpthread

//快速上手
int main()
{
    //提供的编译服务，打包形成一个网络服务
    //cpp-httplib
    Server svr;
    svr.set_base_dir("./wwwroot");//设置首页仅测试
    //http的get请求,匹配的是/hello,就会执行后面的表达式
    svr.Get("/hello",[](const Request& req,Response &resp){
        resp.set_content("hello,你好 httplib","text/plain;charset=utf-8");//设置响应内容 和 格式
    });
    svr.listen("0.0.0.0",8080);//启动http服务
    return 0;
}
```

把编译运行功能打包成网络服务的compile_server.cc实现:

```
void Usage(std::string proc)
{
    std::cerr << "Usage: " << proc << " port" << std::endl; 
}
//compile_server可以被多个主机部署,在一台主机上也能部署多个,需要把端口号暴露出来，运行服务时，手动绑定端口号
//./compile_server 端口号
//例如：./compile_server 8080
int main(int argc,char* argv[])
{
    if(argc != 2){
        Usage(argv[0]);
        return 1;
    }
    //提供的编译服务，打包形成一个网络服务
    //cpp-httplib
    Server svr;
    //http的post请求
    svr.Post("/compile_and_run",[](const Request& req,Response &resp)
    {
        //用户的请求正文是我们想要的json string
        std::string in_json = req.body;
        std::string out_json;
        if(!in_json.empty())
        {
            CompileAndRun::Start(in_json,&out_json);
            //将json格式的字符串响应给用户
            resp.set_content(out_json,"application/json");
        }
    });
    
    svr.listen("0.0.0.0",atoi(argv[1]));//启动http服务
    return 0;
}
```

使用postman进行测试：

在不同端口启动3个服务

![image-20250222232929997](assets/image-20250222232929997.png)

![image-20250222233732775](assets/image-20250222233732775.png)

![image-20250222233822407](assets/image-20250222233822407.png)

### 其他：日志功能

```c++
    //日志等级
    enum{
        INFO,//常规   0
        DEBUG,//调试  1
        WARNING,//警告  2
        ERROR,//当前用户的请求发生错误 3
        FATAL//系统级错误 4
    };
    //LOG(INFO) << "message" << endl
    inline static std::ostream& Log(const std::string &level,const std::string& fileName,int line)
    {
        //添加日志等级
        std::string message = "[";
        message += level;
        message += "]";

        //添加报错文件名称
        message += "[";
        message += fileName;
        message += "]";

        //添加报错行
        message += "[";
        message += std::to_string(line);
        message += "]";

        //添加日志时间戳
        message += "[";
        message += TimeUtil::GetTimeStamp();
        message += "]";

        //cout内部本质是包含缓冲区的，所以要把刚刚的message写入到cout里
        std::cout << message;//不要endl进行刷新
        return std::cout;
    }
    //开放式日志
    #define LOG(level) Log(#level,__FILE__,__LINE__)
```

