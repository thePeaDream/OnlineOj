#include <iostream>
#include "../comm/httplib.h"
#include "oj_control.hpp"
using namespace httplib;
using namespace ns_control;
//获取用户的http请求，通过control来完成对路由功能的解耦
int main()
{
    //访问特定资源的功能路由
    Server svr;
    Control ctrl;
    //1 获取所有的题目列表
    svr.Get("/all_questions",[&ctrl](const Request&req,Response& resp){
        //返回一张包含有所有题目的html网页
        std::string html;
        ctrl.GetAllQuestions(&html);//获取所有题目数据，构建成一个网页,写到html中
        resp.set_content(html,"text/html; charset=utf-8");
    });

    //2 根据题目编号，获取题目内容
    //"/question/100" \d代表匹配数字 +代表1个或多个
    // R"(原始字符串)" ,保持字符串内容的原貌,不用做相关转义
    svr.Get(R"(/question/(\d+))",[&ctrl](const Request& req,Response& resp){
        //把正则匹配到的内容填进来
        std::string number = req.matches[1];
        
        std::string html;
        //将题号、题目、预设代码等拼接成一个网页，返回给用户
        ctrl.GetQuestion(number,&html);
        resp.set_content(html,"text/html;charset=utf-8");
    });

    //3 用户提交代码，使用我们的判题功能(1 每道题的测试用例 2 compile_and_run)
    svr.Get(R"(/judge/(\d+))",[](const Request& req,Response& resp){
        std::string number = req.matches[1];
        resp.set_content("这是指定题目的判题："+number,"text/plain;charset=utf-8");
    }); 
    svr.set_base_dir("./wwwroot");
    svr.listen("0.0.0.0",8080);
    return 0;
}