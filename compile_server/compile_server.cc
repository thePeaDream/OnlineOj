#include "compile_run.hpp"
using namespace ns_compile_and_run;
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


int main()
{
    // testCompile("code");
    // testExecute("code");

    //通过http 让client给我们上传一个json string数据
    std::string in_json;
    Json::Value in_value;
    //in_value["code"] = "#include<iostream> \n int main(){\n std::cout<<\"你好\"<<std::endl \n return 0;}";
    //R"()",raw string,在括号里的字符串，如果出现特殊字符，保持原貌
    in_value["code"] = R"(#include<iostream>
    int main()
    {
        std::cout<<"你好"<<std::endl; 
        return 0;
    })";
    in_value["input"] = "";
    in_value["cpu_limit"] = 1;
    in_value["mem_limit"] = 1024 * 100;//单位是KB，所以空间大小为100M
    
    Json::StreamWriterBuilder writer;
    writer.settings_["emitUTF8"] = true;

    in_json = Json::writeString(writer,in_value);
    

    std::string out_json;//将来给客户端浏览器返回的字符串
    CompileAndRun::Start(in_json,&out_json);

    std::cout << out_json << std::endl;
    return 0;
}
