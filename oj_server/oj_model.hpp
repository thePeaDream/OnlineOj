#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <assert.h>
#include <fstream>
#include <stdlib.h>
#include "../comm/log.hpp"
#include "../comm/util.hpp"

//根据question.list文件，加载题目信息到内存中
//和数据进行交互，对外提供访问数据的接口
namespace ns_model
{
    using namespace ns_util;
    using namespace ns_log;
    using namespace std;
    const std::string question_list = "./questions/question.list";
    const std::string question_path = "./questions/";
    struct Question{
        std::string number;//题目编号
        std::string title;//题目标题
        std::string star;//难度
        int cpu_limit;//时间要求
        int mem_limit;//空间要求
        std::string desc;//题目描述
        std::string header;//题目预设代码
        std::string tailer;//题目的测试用例代码,需要和header拼接，交给后端一起编译
    };
    class Model
    {
    private:
        //题号：题目细节
        std::unordered_map<string,Question> questions;
    public:
        Model()
        {
            assert(LoadQuestionList(question_list));
        }
        ~Model(){}
        bool LoadQuestionList(const std::string& question_list)
        {   
            //根据题目列表的路径，加载配置文件到map里:questions/questions.list 和 题目编号的文件夹
            ifstream in(question_list);
            if(!in.is_open())
            {
                LOG(FATAL) << "加载题库失败,检查题库文件是否存在"<<std::endl;
                return false;
            }
            std::string line;
            while(getline(in,line))
            {
                //拿到一行，切分
                //题号 题目 难度 时间复杂度(s) 空间复杂度(KB)
                vector<string> vs;
                StringUtil::SpiltString(line,&vs," ");
                if(vs.size() != 5)
                {
                    //当前行的切分有误
                    LOG(WARNING) << "部分题目的文件格式有误"<<std::endl;
                    continue;
                }
                Question q;
                q.number = vs[0];
                q.title = vs[1];
                q.star = vs[2];
                q.cpu_limit =atoi(vs[3].c_str());
                q.mem_limit =atoi(vs[4].c_str());
                string path = question_path + q.number + "/";
                string desc = path + "desc.txt";
                string hander = path + "hander.cpp";
                string tailer = path+ "tail.cpp";
                FileUtil::ReadFile(desc,&(q.desc),true);
                FileUtil::ReadFile(hander,&(q.header),true);
                FileUtil::ReadFile(tailer,&(q.tailer),true);

                questions.insert({q.number,q});
            }
            LOG(INFO) << "加载题库成功" << std::endl;
            in.close();
            return true;
        }
        bool GetAllQuestion(vector<Question>*out)
        {
            if(questions.size() == 0)
            {
                LOG(ERROR) << "用户获取题库失败" << std::endl;
                return false;
            }
            for(const auto& q: questions)
            {
                out->push_back(q.second);
            }
            return true;
        }
        bool GetOneQuestion(const std::string& number,Question* q)
        {
            const auto& iter = questions.find(number);
            if(iter != questions.end())
            {
                (*q) = iter->second;
                return true;
            }
            LOG(ERROR) << "用户获取题目失败，对应题目编号为:"<< number << endl; 
            return false;
        }
    };
}

