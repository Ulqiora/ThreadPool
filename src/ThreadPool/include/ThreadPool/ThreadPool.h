#pragma once
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <unordered_map>
#include "Task.h"
class ThreadPool{
public:
    explicit ThreadPool(uint16_t);
    void run();
public:
    template<class Fn, class... Args>
    requires std::invocable<Fn,Args...>
    Help::TaskID addTask(Fn function, Args... args) {
        const Help::TaskID taskId = lastID_++;
        std::unique_lock locker{taskQueueM_};
        taskQueue_.emplace(taskId,Help::TaskDescription{},function,std::forward<Args>(args)...);
        taskQueueCV_.notify_one();
        return taskId;
    }
public:
    std::any waitResult(Help::TaskID id);
    void wait(Help::TaskID id);

    template<class ResultType>
    void waitResult(Help::TaskID id, ResultType& result){
        std::unique_lock locker(taskCompletedListM_);
        taskCompletedListCV_.wait(locker,[this, id](){
            return completedTask_ > id && taskCompletedList_[id].second == Help::Task::Status::kCOMPLETED;
        });
        try {
            result = std::any_cast<ResultType>(taskCompletedList_[id].first);
        }catch (...){
            throw Help::TaskCastException("Incorrect cast result!");
        }
    }

    template<class ResultType>
    ResultType waitResult(Help::TaskID id){
        std::unique_lock locker(taskCompletedListM_);
        taskCompletedListCV_.wait(locker,[this, id](){
            return completedTask_ > id && taskCompletedList_[id].second == Help::Task::Status::kCOMPLETED;
        });
        try {
            return std::any_cast<ResultType>(taskCompletedList_[id].first);
        }catch (...){
            throw Help::TaskCastException("Incorrect cast result!");
        }
    }

    void waitAll();
    ~ThreadPool(){
        quite_ = true;
        taskQueueCV_.notify_all();
        for (auto & thread : threads_) {
            thread.join();
        }
    }
private:
    std::vector<std::thread> threads_;

    std::queue<Help::Task> taskQueue_;
    std::mutex taskQueueM_;
    std::condition_variable taskQueueCV_;

    std::unordered_map<Help::TaskID, std::pair<std::any,Help::Task::Status>> taskCompletedList_;
    std::mutex taskCompletedListM_;
    std::condition_variable taskCompletedListCV_;

    std::atomic_uint64_t lastID_;
    std::atomic_uint64_t completedTask_;
    std::atomic_bool quite_{false};

    std::condition_variable wait_all_cv;
};
