#include <string>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "../comm/util.hpp"
#include "../comm/log.hpp"
namespace ns_runner{
    using namespace ns_util;
    using namespace ns_log;
    class Runner
    {
    public:
        //指名文件名，不需要带路径和后缀
        //0：表示程序正常运行完成，没有发生异常
        //>0：表示程序运行时收到了信号，程序异常,

        static int Run(const std::string&file_name)
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
                LOG(ERROR) << "打开文件失败" << std::endl;
                return -1;
            }
            pid_t id = fork();
            if(id < 0)
            {
                close(stdin_fd);
                close(stdout_fd);
                close(stderr_fd);
                LOG(ERROR) << "创建子进程失败" << std::endl;
                return -2;
            }
            else if(id == 0)//子进程
            {
                //把程序运行的标准输入、输出、错误重定向到临时文件中
                dup2(stdin_fd,0);
                dup2(stdout_fd,1);
                dup2(stderr_fd,2);
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
                return ret & 0x7f;
            }
        }
    };
}