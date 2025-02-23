#include <iostream>
#include <vector>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <signal.h>
#include <jsoncpp/json/json.h>
#include <boost/algorithm/string.hpp>

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
// int main()
// {
    
//     //序列化工作
//     //将结构化的数据，转换成一个字符串
//     //Value是一个Json的中间类，可以填充KV值
//     Json::Value root;
//     root["code"] = "mycode"; 
//     root["user"] = "李勇豪";
//     root["age"] = "18";

//     //用来进行序列化工作的类对象
//     //Json::StyledWriter writer;
//     //Json::FastWriter writer; 中文会乱码
//     Json::StreamWriterBuilder writer;
//     writer.settings_["emitUTF8"] = true;
//     std::string str = Json::writeString(writer,root);
//     std::cout << str << "\n";
//     return 0;
// }

//3 boost库的字符串切分
// int main()
// {
//     std::string str = "1,,判断回文数,简单,1,30000";
//     std::vector<std::string> tokens;
//     const std::string sep = ",";
//     //token_compress_on分割符压缩
//     boost::split(tokens,str,boost::is_any_of(sep),boost::algorithm::token_compress_off);//取消分割符压缩
//     for(auto& iter:tokens)
//     {
//         std::cout << iter << std::endl;
//     }
//     return 0;
// }


//1 需要一个字典，保存数据
/*
{
    "key1":"value",
    "key2":"value2"
}
*/
//2 待被渲染的网页内容
/*
<p>{{key1}}</p>  
<p>{{key2}}</p>
*/
//3 把value替换进{{key1}}.就是渲染
//4 ctemplate库的基本使用
#include <ctemplate/template.h>
int main()
{
    std::string in_html = "./test.html";
    std::string value1 = "你好世界1";
    std::string value2 = "你好世界2";


    //unordered_map<> test
    ctemplate::TemplateDictionary dict("test");
    //test.insert({"key",value})
    dict.SetValue("key1",value1);
    dict.SetValue("key2",value1);
    //获取被渲染网页对象，保持原貌
    ctemplate::Template* tpl = ctemplate::Template::GetTemplate(in_html,ctemplate::DO_NOT_STRIP);
    //添加字典数据到网页中,渲染完成的网页内容放到out_html
    std::string out_html;
    tpl->Expand(&out_html,&dict);
    //渲染结果
    std::cout << out_html << std::endl;
    return 0;
}