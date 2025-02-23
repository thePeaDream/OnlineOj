#ifndef COMPILER_ONLINE//编译时要裁剪掉g++ -D COMPILER_ONLINE
#include "hander.cpp"
#endif


void Test1()
{
    //定义临时对象，完成方法的调用
    bool flag = Solution().isPalindrome(121);
    if(flag){
        std::cout << "通过用例1，测试121通过" << std::endl;
    }
    else{
        std::cout << "没有通过用例1，测试的值是121"<<std::endl;
    }
}
void Test2()
{
    bool flag = Solution().isPalindrome(-233);
    if(!flag){
        std::cout << "通过用例2，测试-233通过" << std::endl;
    }
    else{
        std::cout << "没有通过用例2，测试的值是-233"<<std::endl;
    }
}
//测试用例
int main()
{
    Test1();
    Test2();
    return 0;
}