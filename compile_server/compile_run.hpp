#pragma once
#include "compiler.hpp"
#include "runner.hpp"
#include <jsoncpp/json/json.h>
#include "../comm/log.hpp"
#include "../comm/util.hpp"
//整合，在内部即调用compile.hpp 也调用 runner.hpp
namespace ns_compile_and_run
{
    using namespace ns_log;
    using namespace ns_util;
    using namespace ns_compiler;
    using namespace ns_runner;
    class CompileAndRun
    {
    public:
        //code > 0 进程收到信号导致进程退出/运行时报错
        //code < 0 非运行的报错(代码为空/编译报错/内部错误)
        //code = 0 成功
        static std::string CodeDesc(int code,const std::string& file_name)
        {
            std::string desc;
            switch(code)
            {
            case 0:
                desc = "编译运行成功";
                break;
            case -1:
                desc = "代码为空";
                break;
            case -2:
                desc = "未知错误/内部错误";
                break;
            case -3:
                //desc = "编译发生错误";
                FileUtil::ReadFile(PathUtil::CompilerError(file_name),&desc,true);
                break;
            case SIGABRT://6
                desc = "空间复杂度过高";
                break;
            case SIGXCPU://24
                desc = "时间复杂度过高";
                break;
            case SIGFPE://8
                desc = "浮点数溢出/除0错误";
                break;
            default:
                desc = "收到信号：" + std::to_string(code);
                break;
            }
            return desc;
        }
        /*
        json字符串里的key
        输入：
        code: 用户提交的代码
        input:用户给自己提交的代码对应的输入,方便扩展
        cpu_limit:时间要求   单位s
        mem_limit:空间要求   单位KB
        
        输出：
        必填字段
        status：状态码 0表示成功 <0发生错误
        reason：请求结果
        选填字段
        stdout：用户代码运行完的结果
        stderr：用户代码运行完的错误结果

        参数：
        in_json: {"code":"","input":"","cpu_limit":1,"mem_limit":1024}
        
        out_json:{"status":"0","reason":"运行未发生错误","stdout":"...","stderr":"..."}

        */
        static void Start(const std::string& in_json, std::string *out_json)
        {
            //反序列化：将1个字符串解析成多个kv值
            Json::Value in_value;
            Json::Reader reader;
            reader.parse(in_json,in_value);

            std::string code = in_value["code"].asString();
            std::string input = in_value["input"].asString();
            int cpu_limit = in_value["cpu_limit"].asInt();
            int mem_limit = in_value["mem_limit"].asInt();

            int status_code = 0;
            Json::Value out_value;
            std::string file_name;//形成的唯一文件名
            int run_code = 0; //代码运行的返回值

            //在goto 和 END 之间不能存在定义
            if(code.size() == 0)
            {
                status_code = -1;//代码为空
                goto END;
            }
            file_name = FileUtil::UniqFileName();
            if(!FileUtil::WriteFile(PathUtil::Src(file_name),code))
            {
                status_code = -2;//未知错误/内部错误
                goto END;
            } 

            if(!Compiler::Compile(file_name))
            {
                status_code = -3;//编译失败
                goto END;
            }
            run_code = Runner::Run(file_name,cpu_limit,mem_limit);
            if(run_code < 0)
            {
                status_code = -2;//未知错误/内部错误
            }
            else if(run_code > 0)
            {
                //程序运行崩溃，收到信号
                status_code = run_code;
            }
            else
            {
                //运行成功
                status_code = 0;
            }
        END:
            out_value["status"] = status_code;
            out_value["reason"] = CodeDesc(status_code,file_name);
            if(status_code == 0)
            {
                std::string std_out;
                std::string std_err;
                FileUtil::ReadFile(PathUtil::Stdout(file_name),&std_out,true);
                FileUtil::ReadFile(PathUtil::Stderr(file_name),&std_err,true);
                //整个过程全部成功，才需要填充stdout
                out_value["stdout"] = std_out;
                out_value["stderr"] = std_err;
            }
            //反序列化
            Json::StreamWriterBuilder writer;
            writer.settings_["emitUTF8"] = true;
            *out_json = Json::writeString(writer,out_value);

            //把结果全部拿到以后，清理所有相关临时文件
            FileUtil::RemoveTempFile(file_name);


            // if(code.size() == 0)
            // {
            //     out_value["status"] = -1;//代码为空
            //     out_value["reason"] = "用户代码为空";
            //     //序列化
            //     return;
            // }
            // //1 生成唯一文件名，形成的文件名只具有唯一性，没有目录，没有后缀
            // //用毫秒级时间戳+原子性递增唯一值，保证唯一性
            // std::string file_name = FileUtil::UniqFileName();

            // //2 把用户代码写到该文件中,形成临时src文件
            // if(!FileUtil::WriteFile(PathUtil::Src(file_name),code))
            // {
            //     out_value["status"] = -2;//未知错误
            //     out_value["reason"] = "发生未知错误";
            //     //序列化
            //     return;
            // }

            // if(!Compiler::Compile(file_name))
            // {
            //     //编译失败
            //     out_value["status"] = -3;//代码编译时发生错误
            //     out_value["reason"] = FileUtil::ReadFile(PathUtil::CompilerError(file_name));
            //     //读取 编译失败生成的临时文件.compile_error，将具体编译错误返回
            //     return;
            // }

            // int retCode = Runner::Run(file_name,cpu_limit,mem_limit);
            // if(retCode < 0 )
            // {
            //     out_value["status"] = -2;//未知错误 服务器
            //     out_value["reason"] = "发生未知错误";
            //     //序列化
            //     return;
            // }
            // else if(retCode > 0)//用户代码出问题，运行时报错
            // {
            //     out_value["status"] = retCode;//运行时报错
            //     out_value["reason"] = SignalDesc(retCode);//将信号转换成报错原因
            // }
        }

    };
}