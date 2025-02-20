#include "compiler.hpp"
using namespace ns_compiler;
//测试编译功能
void testCompile()
{
    std::string code = "code";
    Compiler::Compile(code);
}

int main()
{
    testCompile();
    return 0;
}
