#pragma once
#include <iostream>
#include <string>
#include <atomic>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <fstream>

namespace ns_util
{
    const std::string temp_path = "./temp/";
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

        //获取毫秒级别时间戳
        static std::string GetTimeMs()
        {
            struct timeval tv;
            //time_t tv_sec   //秒
            //suseconds_t tv_usec  //微秒
            gettimeofday(&tv,nullptr);
            return std::to_string(tv.tv_usec / 1000 + tv.tv_sec * 1000);
        }
    };

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

        //生成一个唯一的文件名,没有后缀和目录
        //毫秒级时间戳+原子性递增唯一值：保证唯一性
        static std::string UniqFileName()
        {
            //原子性递增的计数器
            static std::atomic_uint id(0);
            std::string ms = TimeUtil::GetTimeMs();
            std::string uniq_id = std::to_string(id);
            id++;
            return ms + "xx" + uniq_id;
        }
        
        //把用户代码写到该文件中,形成临时src文件
        static bool WriteFile(const std::string& path_name,const std::string& content)
        {
            std::ofstream out(path_name);
            if(!out.is_open())
            {
                return false;
            }
            out.write(content.c_str(),content.size());

            out.close();
            return true;
        }

        //读取文件
        static bool ReadFile(const std::string& path_name,std::string* content/*输出型参数*/,bool keep = false/*是否保留文件里的\n*/)
        {
            (*content).clear();
            std::ifstream in(path_name);
            if(!in.is_open())
            {
                return false;
            }
            std::string line;
            //getline不保存行分割符，读上来时会去掉\n，但有时候需要保留\n
            //getline内部重载了强制类型转化
            while(std::getline(in,line))
            {
                (*content) += line;
                (*content) += (keep ? "\n":"");
            }

            in.close();
            return true;
        }

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

    };
    
}