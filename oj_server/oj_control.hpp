#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "../comm/util.hpp"
#include "../comm/log.hpp"
#include "../comm/httplib.h"
#include "oj_model.hpp"
#include "oj_view.hpp"
#include <jsoncpp/json/json.h>
#include <mutex>
#include <fstream>
#include <assert.h>

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
        //递增负载
        void IncreaseLoad()
        {
            if(mtx) mtx->lock();
            ++load;
            if(mtx) mtx->unlock();
        }
        //递减负载
        void decreaseLoad()
        {
            if(mtx) mtx->lock();
            --load;
            if(mtx) mtx->unlock();
        }
        //获取主机负载
        uint64_t Load()
        {
            uint64_t _load = 0;
            if(mtx) mtx->lock();
            _load = load;
            if(mtx) mtx->unlock();
            return _load;
        }
        void ResetLoad()
        {
            if(mtx) mtx->lock();
            load = 0;
            if(mtx) mtx->unlock();
        }
    };
    //负载均衡模块
    const std::string service_machine = "./conf/service_machine.conf";
    class LoadBlance
    {
    private:
        //每一台主机都有自己的下标，充当当前主机的id
        std::vector<Machine> machines;//可以给我们提供编译服务的所有主机
        //所有在线的主机id
        std::vector<int> online;
        //所有离线的主机id
        std::vector<int> offline;
        // 保证LoadBlance的数据安全
        std::mutex mtx;
    public:
        LoadBlance()
        {
            //在进行负载均衡之前，所有的主机信息必须先加载进来
            assert(LoadConf(service_machine));
            LOG(INFO) << "加载配置文件"+ service_machine +"成功" << endl;
        }
        ~LoadBlance() {}
        //加载配置文件
        bool LoadConf(const std::string& machine_conf)
        {
            std::ifstream in(machine_conf);
            if(!in.is_open())
            {
                //只能查看题目，不能判题
                LOG(FATAL) << "加载" << machine_conf << "失败" <<"\n";
                return false;
            }
            string line;
            while(getline(in,line))
            {
                vector<string> vs;
                StringUtil::SpiltString(line,&vs,":");
                if(vs.size() != 2)
                {
                    LOG(WARING) << "切分" << line << "失败" << endl;
                    continue;
                }
                Machine m;
                m.ip = vs[0];
                m.port = atoi(vs[1].c_str());
                m.load = 0;
                m.mtx = new mutex();
                online.push_back(machines.size());
                machines.push_back(m);
            }
            in.close();
            return true;
        }

        //根据负载情况选择一台在线主机
        //id:输出型参数
        //m:输出型参数 获取主机的地址
        bool Select(int* id,Machine** m)
        {
            //1 使用选择好的主机(更新该主机的负载)
            //2 后面可能离线该主机
            mtx.lock();
            //负载均衡的算法
            // 轮询+hash
            int online_num = online.size();
            if(online_num == 0)
            {
                mtx.unlock();
                LOG(FATAL) << "所有的后端编译主机已经离线！！！！" << endl;
                return false;
            }
            //遍历online,找到负载最小的主机
            *id = online[0];
            *m = &machines[online[0]];
            uint64_t min_load = machines[online[0]].Load();
            for(int i = 1; i < online_num; ++i)
            {
                uint64_t cur_load = machines[online[i]].Load();
                if(min_load > cur_load)
                {
                    min_load = cur_load;
                    *id = online[i];
                    *m = &machines[online[i]];
                }
            }
            mtx.unlock();
            return true;
        }

        //id:要离线的主机id
        void OfflineMachine(int id)
        {
            //在online中找到要离线的主机id
            //将它移除，同时添加到offline中
            mtx.lock();
            auto iter = online.begin();
            while(iter!=online.end()) 
            {
                if(*iter == id)
                {
                    machines[id].ResetLoad();
                    online.erase(iter);
                    offline.push_back(id);
                    break;
                }
                ++iter;
            }
            mtx.unlock();
        }
        void OnlineMachine()
        {
            //当所有主机都离线时，统一上线
            mtx.lock();
            online.insert(online.end(),offline.begin(),offline.end());
            offline.erase(offline.begin(),offline.end());
            mtx.unlock();

            LOG(INFO) << "所有主机重新上线" << endl;
        }

        void ShowMachines()
        {
            mtx.lock();
            cout << "当前在线主机列表" << endl;
            for(auto& id : online)
            {
                cout << machines[id].ip + ":" + to_string(machines[id].port) << endl;
            }

            cout << "当前离线主机列表" << endl;
            for(auto& id : offline)
            {
                cout << machines[id].ip + ":" + to_string(machines[id].port) << endl;
            }
            cout << endl;
            mtx.unlock();
        }
    };

    //核心业务逻辑控制器
    class Control
    {
    public:
        Control(){}
        ~Control(){}
        bool GetAllQuestions(string* html)//获取所有题目数据，构建成一个网页,写到html中
        {
            bool flag = true;
            vector<Question> all;
            if(_model.GetAllQuestion(&all))
            {
                sort(all.begin(),all.end(),[](const Question& q1,const Question& q2){
                    return atoi(q1.number.c_str()) < atoi(q2.number.c_str());// < 是升序排序
                });
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
        void Judge(const string& number, const string in_json,string* out_json)
        {
            //1 根据题号，得到题目详情
            Question q;
            _model.GetOneQuestion(number,&q);
            //2 反序列化in_json,得到用户源代码、input
            Json::Reader reader;
            Json::Value in_value;
            reader.parse(in_json,in_value);
            string code = in_value["code"].asString();
            //3 根据题号拼接用户代码+测试用例代码
            Json::Value compile_value;
            compile_value["input"] = in_value["input"].asString();
            compile_value["code"] = code +"\n" + q.tailer;
            compile_value["cpu_limit"] = q.cpu_limit;
            compile_value["mem_limit"] = q.mem_limit;
            // 将要发送给主机的内容序列化成字符串
            Json::StreamWriterBuilder writer;
            writer.settings_["emitUTF8"] = true;
            string compile_json = Json::writeString(writer,compile_value);
            //4 选择负载最低的主机
            //规则：一直选择，直到主机可用；
            //特殊：全部主机都无法使用
            while(true)
            {
                int id = 0;
                Machine *m = nullptr;
                if(!load_blance.Select(&id,&m))
                {
                    //所有主机都挂掉了
                    break;
                }
                
                //5 发起http请求，得到结果
                httplib::Client cli(m->ip,m->port);

                m->IncreaseLoad();//开始请求之前，增加负载
                LOG(INFO) << "选择主机id:" << id << "详情：" << m->ip << ":" << m->port << endl; 
                LOG(INFO) << "当前主机负载情况："<< m->Load() << endl;
                if(auto res = cli.Post("/compile_and_run",compile_json,"application/json;charset=utf-8"))
                {
                    //6 只要是正常获取编译结果，将结果交给 *out_json
                    //否则重新选择主机 
                    if(res->status == 200)
                    {
                        *out_json = res->body;
                        m->decreaseLoad();//请求完毕，减少负载
                        LOG(INFO) << "请求编译和运行服务成功........" <<endl;
                        break;
                    }
                    else
                        m->decreaseLoad();//请求完毕,但状态码不是200，仍然需要减少负载
                }
                else
                {
                    //请求失败，当前请求的主机可能离线，去选别的主机
                    LOG(ERROR)<<"当前请求的主机id:" << id <<"详情："<<m->ip << ":" << m->port << "可能离线了" << endl;
                    //离线会把负载清0的
                    load_blance.OfflineMachine(id);
                    //load_blance.ShowMachines();//调试
                }
            }
        }
        void RecoveryMachine()
        {
            load_blance.OnlineMachine();
        }
    private:
        //要有对数据的操作对象才能逻辑控制
        Model _model;
        //视图用于网页的数据渲染
        View _view;
        LoadBlance load_blance;//核心负载均衡器
    };
}