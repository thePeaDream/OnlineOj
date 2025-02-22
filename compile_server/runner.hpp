#include <string>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "../comm/util.hpp"
#include "../comm/log.hpp"
#include <sys/time.h>
#include <sys/resource.h>
namespace ns_runner{
    using namespace ns_util;
    using namespace ns_log;
    class Runner
    {
    public:
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
            //父进程           
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
    };
}