#pragma once
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
namespace ns_util
{
    const std::string temp_path = "./temp/";
    //路径拼接类工具
    class PathUtil
    {
    public:
        //添加前缀后缀
        static std::string AddSuffix(const std::string & file_name,const std::string& suffix)
        {
            std::string path_name = temp_path;
            path_name += file_name;
            path_name += suffix;
            return path_name;
        }

        //编译时需要有的临时文件
        //构建源文件路径+后缀
        //1234 -> ./temp/1234.cpp
        static std::string Src(const std::string &file_name)
        {
           return AddSuffix(file_name,".cpp");
        }
        //构建可执行程序的完整路径+后缀名
        static std::string  Exe(const std::string &file_name)
        {
            return AddSuffix(file_name,".exe");
        }
        //编译时报错形成的文件
        static std::string CompilerError(const std::string &file_name)
        {
            return AddSuffix(file_name,".compile_error");
        }

        //运行时需要的临时文件
        static std::string Stdin(const std::string &file_name)
        {
            return AddSuffix(file_name,".stdin");
        }
        static std::string Stdout(const std::string &file_name)
        {
            return AddSuffix(file_name,".stdout");
        }
        //构建该程序对应的标准错误完整的路径+后缀名(运行时错误形成的文件)
        static std::string Stderr(const std::string &file_name)
        {
            return AddSuffix(file_name,".stderr");
        }
        
    };
    //文件工具类
    class FileUtil
    {
    public:
        //判断文件是否存在
        static bool IsFileExists(const std::string& path_name)
        {
            //int stat(const char* path,struct stat* buf)第二个参数是输出型参数，获取文件的属性
            //获取文件属性成功返回0；失败返回-1
            struct stat st;
            int ret = stat(path_name.c_str(),&st);
            if(ret == 0) return true;
            return false;
        }
    };
    //时间工具类
    class TimeUtil
    {
    public:
        static std::string GetTimeStamp()
        {
            // int gettimeofday(struct timeval* tv,struct timezone *tz//时区设置为nullptr)
            //tv:输出型参数
            struct timeval tv;
            gettimeofday(&tv,nullptr);
            return std::to_string(tv.tv_sec);
        }
    };
}