#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "../comm/util.hpp"
#include "../comm/log.hpp"
#include "oj_model.hpp"
#include "oj_view.hpp"
#include <jsoncpp/json/json.h>
#include <mutex>

namespace ns_control
{
    using namespace std;
    using namespace ns_log;
    using namespace ns_util;
    using namespace ns_model;
    using namespace ns_view;
    
    //提供编译运行服务的主机
    struct Machine
    {
        string ip; //编译服务的ip
        int port;  //端口号
        uint64_t load;  //负载情况
        std::mutex *mtx;//mutex禁止拷贝，如果Machine被放在某个容器里,一定会发生拷贝，使用指针
        Machine()
        :ip("")
        ,port(0)
        ,load(0)
        ,mtx(nullptr)
        {}
        ~Machine(){}
    };
    //负载均衡模块
    class LoadBlance
    {
    private:
        //每一台主机都有自己的下标，充当当前主机的id
        std::vector<Machine> machines;//可以给我们提供编译服务的所有主机
        //所有在线的主机id
        std::vector<int> online;
        //所有离线的主机id
        std::vector<int> offline;
    };



    //核心业务逻辑控制器
    class Control
    {
    public:
        Control(){}
        ~Control(){}
        bool GetAllQuestions(std::string* html)//获取所有题目数据，构建成一个网页,写到html中
        {
            bool flag = true;
            vector<Question> all;
            if(_model.GetAllQuestion(&all))
            {
                //获取题目信息成功，用View将所有的题目数据构建成网页
                _view.AllQuestionHtml(all,html);
            }
            else
            {
                *html = "获取题目失败，形成题目网页失败";
                flag = false;
            }
            return flag;
        }
        bool GetQuestion(const std::string& number,std::string *html)
        {
            bool flag = true;
            Question q;
            if(_model.GetOneQuestion(number,&q))
            {
                //获取指定题目成功
                _view.OneQuestionHtml(q,html);

            }
            else
            {
                *html = "获取指定题目："+ number + "不存在！！！";
                flag = false;
            }
            return flag;
        }
        //id:100
        //code:""
        //input:""
        void Judge(const std::string in_json,std::string* out_json)
        {
            //1 反序列化in_json,得到题目id、用户源代码、input
            //2 拼接用户代码+测试用例代码
            //3 选择负载最低的主机
            //4 发起http请求，得到结果
            //5 将结果交给 *out_json 
        }
    private:
        //要有对数据的操作对象才能逻辑控制
        Model _model;
        //视图用于网页的数据渲染
        View _view;
    };
}