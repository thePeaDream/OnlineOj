#include <iostream>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <signal.h>
#include <jsoncpp/json/json.h>
//1 没有做相关的资源约束，一直在死循环
// int main()
// {
//     while(1);
//     return 0;
// }

// void handler(int signal)
// {
//     std::cout << "收到信号："<<signal << "\n";
//     exit(1);
// }
// int main()
// {
//     //资源不总，导致os终止进程，是通过信号终止的 
//     for(int i = 1; i <= 31; ++i)
//     {
//         signal(i,handler);
//     }

//     //限制累计运行时长
//     struct rlimit r;
//     r.rlim_cur = 1;//最多1秒
//     r.rlim_max = RLIM_INFINITY;//硬上限设置为无穷大
//     setrlimit(RLIMIT_CPU,&r);//设置资源约束
//     while(1);
    
//     // //限制空间大小
//     // struct rlimit r;
//     // //注意：程序本身加载也会占用空间
//     // r.rlim_cur = 1024*1024*40;//40M
//     // r.rlim_max = RLIM_INFINITY;
//     // setrlimit(RLIMIT_AS,&r);
//     // int count = 0;
//     // while(true)
//     // {
//     //     int* p = new int[1024*1024];//1M
//     //     count++;
//     //     std::cout << "申请空间：" << count << " M" << std::endl;
//     //     sleep(1);
//     // }
//     return 0;
// }

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