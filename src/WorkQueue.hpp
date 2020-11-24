#pragma once

#include <functional>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

class WorkQueuePriority {
    public:
        WorkQueuePriority(
            std::function<
                void(
                    std::mutex *,
                    std::condition_variable *,
                    std::deque<std::function<void(void)>> *,
                    std::mutex *,
                    int *,
                    int
                )
            > handler,
            int number
        );

        std::string debug();
        void addWork(std::function<void(void)> work);
        bool addWork(std::function<void(void)> work, bool force);

        ~WorkQueuePriority();
    protected:
        std::vector<std::thread *> threads;
        std::mutex queue_mutex;
        std::deque<std::function<void(void)>> *queue;
        std::condition_variable condition;

        std::mutex count_available_mutex;
        int count_available;
    private:
};

class WorkQueue {
    public:
        WorkQueue(int dedicatedHigh, int dedicatedMediumPlus, int low);

        void addHighWork(std::function<void(void)> work);
        void debug();

        ~WorkQueue();
    protected:
        WorkQueuePriority *high;
        WorkQueuePriority *medium;
        WorkQueuePriority *low;
    private:
};