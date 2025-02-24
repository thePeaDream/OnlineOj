#ifndef COMPILER_ONLINE//编译时要裁剪掉g++ -D COMPILER_ONLINE
#include "hander.cpp"
#endif

void Test1()
{
    std::vector<int> v = {1,2,3,8,0};
    int max = Solution().Max(v);
    if(max == 8){
        cout << "用例1通过"<<endl;
    }else{
        cout << "用例1不通过"<<endl;
    }
}
void Test2()
{
    std::vector<int> v = {88,-1,-3,-5};
    int max = Solution().Max(v);
    if(max == 88){
        cout << "用例2通过"<<endl;
    }
    else{
        cout << "用例2不通过"<<endl;
    }
}

int main()
{
    Test1();
    Test2();
    return 0;
}