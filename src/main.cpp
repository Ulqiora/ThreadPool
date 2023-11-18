#include <iostream>
#include <ThreadPool/ThreadPool.h>
int foo(){
    std::cout<<4;
    return 5;
}
int main(){
    std::cout<<"Hello world!";
    ThreadPool tp(1);
    std::function<void()> func = []() { foo(); };
    Help::TaskID id = tp.addTask(func);
    tp.wait(id);
    return 0;
}
