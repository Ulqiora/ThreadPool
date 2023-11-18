#pragma once
#include <exception>
#include <string_view>
namespace Help {

class TaskCastException final : public std::exception {
public:
    explicit TaskCastException(std::string_view message):message_(message){}
    const char* what(){return message_.data();}
private:
    std::string_view message_;
};

} // Help

