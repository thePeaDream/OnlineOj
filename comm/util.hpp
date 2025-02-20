#pragma once
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
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
        //构建该程序对应的标准错误完整的路径+后缀名
        static std::string  Stderr(const std::string &file_name)
        {
            return AddSuffix(file_name,".stderr");
        }
    };
    //文件工具
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
}