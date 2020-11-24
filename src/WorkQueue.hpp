#pragma once

#include <functional>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

class WorkQueue {
    public:
        WorkQueue(int dedicatedHigh, int dedicatedMediumPlus, int low);

        void addHighWork(  std::function<void(void)> work);
        void addMediumWork(std::function<void(void)> work);
        void addLowWork(   std::function<void(void)> work);

        ~WorkQueue();
    protected:
        std::deque<std::function<void(void)>> *high_queue;
        std::deque<std::function<void(void)>> *medium_queue;
        std::deque<std::function<void(void)>> *low_queue;
        std::mutex high_mutex;
        std::mutex medium_mutex;
        std::mutex low_mutex;
        std::condition_variable high_condition;
        std::condition_variable medium_condition;
        std::condition_variable low_condition;
        std::vector<std::thread *> hi_threads;
        std::vector<std::thread *> medium_threads;
        std::vector<std::thread *> low_threads;
    private:
};