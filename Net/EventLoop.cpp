#include "EventLoop.h"

EventLoop::EventLoop(uint32_t num_threads)
    :num_threads_(num_threads),index_(0)
{
    Loop();
}
EventLoop::~EventLoop()
{
    Quit();
}

TimerId EventLoop::AddTimer(const TimerCallback &callback, uint32_t msec)
{
    return schedulers_[index_]->AddTimer(callback, msec);
}

void EventLoop::RemoveChannel(Channel *channel)
{
    schedulers_[index_]->RemoveChannel(channel);
}

void EventLoop::UpdateChannel(Channel *channel)
{
    schedulers_[index_]->UpdateChannel(channel);
}

void EventLoop::RemoveTimer(TimerId timer_id)
{
    schedulers_[index_]->RemoveTimer(timer_id);
}

void EventLoop::Loop()
{
    if(!schedulers_.empty()){
        return;
    }
    for(uint32_t i = 0; i<num_threads_;++i){
        auto scheduler = std::make_shared<EpollTaskScheduler>(i);// 创建EpollTaskScheduler对象，并传入线程ID
        schedulers_.push_back(scheduler);
        auto thread = std::make_shared<std::thread>([scheduler](){
            scheduler->Start();// 启动线程，开始处理事件循环
        });
        threads_.push_back(thread);// 将线程对象存储到threads_中，确保线程的生命周期与EventLoop对象一致
    }
}

void EventLoop::Quit()
{
    for(auto &scheduler : schedulers_){
        scheduler->Stop();
    }
    for(auto &thread : threads_){
        if(thread->joinable()){
            thread->join();// 等待线程结束，确保所有线程在EventLoop对象销毁前完成
        }
    }
    schedulers_.clear();
    threads_.clear();
}

std::shared_ptr<TaskScheduler> EventLoop::GetTaskScheduler() 
{
    if(schedulers_.empty()){
        return nullptr;
    }
    auto scheduler = schedulers_[index_];
    index_ = (index_ + 1) % num_threads_; // 轮询选择下一个TaskScheduler
    return scheduler;
}