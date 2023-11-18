#include "ThreadPool.h"

    ThreadPool::ThreadPool(uint16_t sizePool) {
        threads_.reserve(sizePool);
        for(uint16_t i=0;i<sizePool;++i){
            threads_.emplace_back(&ThreadPool::run,this);
        }
    }
    void ThreadPool::run() {
        while (!quite_) {
            std::unique_lock locker(taskQueueM_);
            taskQueueCV_.wait(locker,[this](){return !taskQueue_.empty() || quite_;});
            if(!taskQueue_.empty() || quite_){
                auto q = std::move(taskQueue_.front());
                taskQueue_.pop();
                locker.unlock();

                q();
                assert(q.status() == Help::Task::Status::kCOMPLETED);
                std::lock_guard lockerLG(taskCompletedListM_);
                if(q.hasValue()){
                    taskCompletedList_[q.Id()]={q.get_result(),q.status()};
                }
                completedTask_++;

            }
            wait_all_cv.notify_all();
            taskCompletedListCV_.notify_all();
        }
    }

    std::any ThreadPool::waitResult(Help::TaskID id) {
        std::unique_lock locker(taskCompletedListM_);
        taskCompletedListCV_.wait(locker,[this, id](){
            return completedTask_ > id && taskCompletedList_[id].second == Help::Task::Status::kCOMPLETED;
        });
        return taskCompletedList_[id].first;
    }

    void ThreadPool::wait(Help::TaskID id) {
        std::unique_lock locker(taskCompletedListM_);
        taskCompletedListCV_.wait(locker,[this, id](){
            return completedTask_ > id && taskCompletedList_[id].second == Help::Task::Status::kCOMPLETED;
        });
    }
    void ThreadPool::waitAll(){
        std::unique_lock<std::mutex> lock(taskCompletedListM_);
        wait_all_cv.wait(lock, [this]()->bool { return completedTask_ == lastID_; });
    }







//    template<class Fn, class... Args>
////    requires std::invocable<Fn,Args...>
//    Help::TaskID ThreadPool::addTask(Fn function, Args... args) {
//        const Help::TaskID taskId = lastID_++;
//        std::unique_lock locker{taskQueueM_};
//        taskQueue_.emplace(taskId,Help::TaskDescription{},function,args...);
////        taskQueueCV_.notify_one();
//        return taskId;
//    }


