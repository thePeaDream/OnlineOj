#pragma once
//编译服务
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../comm/util.hpp"
#include "../comm/log.hpp"
//只负责进行代码的编译
namespace ns_compiler
{
    //引入路径拼接功能
    using namespace ns_util;
    using namespace ns_log;
    class Compiler{
        public:
        Compiler(){}
        ~Compiler(){}
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
                umask(0);
                int _stderr = open(PathUtil::CompilerError(file_name).c_str(),O_CREAT|O_WRONLY,0644); 
                if(_stderr < 0)
                {
                    LOG(WARNING) << "没有成功形成compile_error文件"<<std::endl;
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
                    LOG(INFO) << PathUtil::Src(file_name)<<"编译成功"<<"\n";
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
    };
}
