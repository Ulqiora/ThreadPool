#pragma once
#include <cstdint>
#include <string>
#include <memory>
#include <concepts>
#include <any>
#include <functional>
#include <cassert>
#include "TaskCastException.h"
class ThreadPool;
namespace Help{
    using TaskID = uint64_t;
    using TaskDescription = std::string;

    template<class T = void>
    struct JustType{
        using Type = T;
    };
    class Task {
    public:
        enum class Status{
            kAWAITING,
            kCOMPLETED
        };

        template<class Fn, class... Args>
        requires std::invocable<Fn,Args...>
        Task(TaskID id, TaskDescription&& description, Fn function, Args... args) :
                status_{Task::Status::kAWAITING},
                id_{id},
                description_(std::move(description)){
            using Type = std::invoke_result_t<Fn,Args...>;
            if constexpr (std::is_void_v<Type>){
                voidFunction_ = std::bind(function,std::forward<Args>(args)...);
                anyFunction_ = []()->std::any{return {};};
            } else {
                voidFunction_ = []()->Type{};
                anyFunction_ = std::bind(function,std::forward<Args>(args)...);
            }
        }

        [[nodiscard]] TaskID Id()const noexcept{
            return id_;
        }

        template<class Fn, class... Args>
        requires std::invocable<Fn,Args...>
        void setFunction(Fn function,Args ...args){
            using Type = std::invoke_result_t<Fn,Args...>;
            if constexpr (std::is_void_v<Type>){
                voidFunction_ = std::bind(function,std::forward<Args>(args)...);
                anyFunction_ = []()->void{};
            } else {
                voidFunction_ = []()->Type{};
                anyFunction_ = std::bind(function,std::forward<Args>(args)...);
            }
        }
        template<class Type>
        Type getType(){
            try {
                return any_cast<Type>(result_);
            } catch (...){
                throw TaskCastException(std::string("Template type with name \"")+ typeid(Type).name()+"\" is not exists!!!");
            }
        }

        void operator()(){
            voidFunction_();
            result_ = anyFunction_();
            status_ = Status::kCOMPLETED;
        }
        [[nodiscard]] Status status() const noexcept{
            return  status_;
        }

        [[nodiscard]] bool hasValue() const{
            return !isVoid_;
        }

        [[nodiscard]] std::any get_result() const {
            assert(!isVoid_);
            return result_;
        }

    protected:
        Status status_;
        TaskID id_{};
        TaskDescription description_;
        std::function<std::any()> anyFunction_;
        std::function<void()> voidFunction_;
        std::any result_;
        bool isVoid_{};
    };
}
