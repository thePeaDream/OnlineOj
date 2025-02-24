#pragma once
#include <iostream>
#include <string>
#include "util.hpp"
//日志
namespace ns_log
{
    using ns_util::TimeUtil;
    //日志等级
    enum{
        INFO,//常规   0
        DEBUG,//调试  1
        WARNING,//警告  2
        ERROR,//当前用户的请求发生错误 3
        FATAL//系统级错误 4
    };

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
}