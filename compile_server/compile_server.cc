#include "compile_run.hpp"
#include "../comm/httplib.h"
using namespace ns_compile_and_run;
using namespace httplib;
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

// using namespace ns_compiler;
// using namespace ns_runner;
// //测试编译模块
// void testCompile(const std::string& file_name)
// {
//     Compiler::Compile(file_name);
// }

// //测试运行模块
// void testExecute(const std::string& file_name)
// {
//     int ret = Runner::Run(file_name,1,20);
//     if(ret == 0) std::cout << "运行成功" << std::endl;
//     else if(ret < 0) std::cout << "内部错误"<<std::endl;
//     else
//         std::cout << "程序异常退出" << std::endl;
// }


// int main()
// {
    
//     //通过http 让client给我们上传一个json string数据
//     std::string in_json;
//     Json::Value in_value;
//     //in_value["code"] = "#include<iostream> \n int main(){\n std::cout<<\"你好\"<<std::endl \n return 0;}";
//     //R"()",raw string,在括号里的字符串，如果出现特殊字符，保持原貌
//     in_value["code"] = R"(#include<iostream>
//     int main()
//     {
//         std::cout<<"你好"<<std::endl; 
//         return 0;
//     })";
//     in_value["input"] = "";
//     in_value["cpu_limit"] = 1;
//     in_value["mem_limit"] = 1024 * 100;//单位是KB，所以空间大小为100M
    
//     Json::StreamWriterBuilder writer;
//     writer.settings_["emitUTF8"] = true;

//     in_json = Json::writeString(writer,in_value);
    

//     std::string out_json;//将来给客户端浏览器返回的字符串
//     CompileAndRun::Start(in_json,&out_json);

//     std::cout << out_json << std::endl;
//     return 0;
// }



