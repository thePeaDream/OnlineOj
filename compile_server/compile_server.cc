#include "compiler.hpp"
#include "runner.hpp"
using namespace ns_compiler;
using namespace ns_runner;
//测试编译模块
void testCompile(const std::string& file_name)
{
    Compiler::Compile(file_name);
}

//测试运行模块
void testExecute(const std::string& file_name)
{
    int ret = Runner::Run(file_name);
    if(ret == 0) std::cout << "运行成功" << std::endl;
    else if(ret < 0) std::cout << "内部错误"<<std::endl;
    else
        std::cout << "程序异常退出" << std::endl;
}

int main()
{
    testCompile("code");
    testExecute("code");
    return 0;
}
