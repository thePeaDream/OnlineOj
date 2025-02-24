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
            resp.set_content(out_json,"application/json;charset=utf-8");
        }
    });
    
    svr.listen("0.0.0.0",atoi(argv[1]));//启动http服务
    return 0;
}


