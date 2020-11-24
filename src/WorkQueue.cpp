#include "WorkQueue.hpp"

#include <iostream>

WorkQueuePriority::WorkQueuePriority(
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
) {
    this->queue = new std::deque<std::function<void(void)>>();

    this->count_available = number;
    for (int i = 0; i < number; i++) {
        this->threads.emplace_back(
            new std::thread(
                std::bind(
                    handler,
                    &this->queue_mutex,
                    &this->condition,
                    this->queue,
                    &this->count_available_mutex,
                    &this->count_available,
                    i
                )
            )
        );
    }
}

std::string WorkQueuePriority::debug() {
    std::string out;
    out += "Count: ";

    std::unique_lock<std::mutex> lock(this->count_available_mutex);
    out += std::to_string(this->count_available);

    return out;
}

bool WorkQueuePriority::addWork(std::function<void(void)> work, bool force) {
    this->count_available_mutex.lock();
    if (this->count_available > 0) {
        this->queue_mutex.lock();
        if (this->queue->size() < this->count_available) {
            this->queue_mutex.unlock();
            this->count_available_mutex.unlock();

            this->addWork(work);

            return true;
        }

        this->queue_mutex.unlock();
        this->count_available_mutex.unlock();

        return false;
    }
    this->count_available_mutex.unlock();

    if (force) {
        this->addWork(work);
    }

    return false;
}

void WorkQueuePriority::addWork(std::function<void(void)> work) {
    std::unique_lock<std::mutex> lock(this->queue_mutex);

    this->queue->emplace_back(work);
    lock.unlock();

    this->condition.notify_one();
}

WorkQueuePriority::~WorkQueuePriority() {
    for (auto &thread: this->threads) {
        thread->join();
        delete thread;
    }
}

/**
 * TODO: The three handlers can share some code. I am not consolidating them
 *       YET because I need to work out how to make the queues do priorities
 *       correctly. Once that's done I can start consolidating.
 */
void hi_handler(
    std::mutex *mutex,
    std::condition_variable *condition_variable,
    std::deque<std::function<void(void)>> *queue,
    std::mutex *count_available_mutex,
    int *count_available,
    int thread_id
) {
    std::unique_lock<std::mutex> lock(*mutex);

    while (true) {
        // NOTE: Because we use the predicate here, if our queue only has a
        //       single item at a time (which is common in artificial test
        //       cases) one worker will become "sticky" and always handle the
        //       work.
        condition_variable->wait(lock, [queue]{
            return (queue->size());
        });

        if (queue->size() <= 0) {
            continue;
        }

        count_available_mutex->lock();
        (*count_available)--;
        count_available_mutex->unlock();

        auto a = queue->front();
        queue->pop_front();
        lock.unlock();
        a();

        count_available_mutex->lock();
        (*count_available)++;
        count_available_mutex->unlock();

        lock.lock();
    }
}

void medium_handler(
    std::mutex *mutex,
    std::condition_variable *condition_variable,
    std::deque<std::function<void(void)>> *queue,
    std::mutex *count_available_mutex,
    int *count_available,
    int thread_id
) {
    std::unique_lock<std::mutex> lock(*mutex);

    while (true) {
        // NOTE: Because we use the predicate here, if our queue only has a
        //       single item at a time (which is common in artificial test
        //       cases) one worker will become "sticky" and always handle the
        //       work.
        condition_variable->wait(lock, [queue]{
            return (queue->size());
        });

        if (queue->size() <= 0) {
            continue;
        }

        count_available_mutex->lock();
        (*count_available)--;
        count_available_mutex->unlock();

        auto a = queue->front();
        queue->pop_front();
        lock.unlock();
        a();

        count_available_mutex->lock();
        (*count_available)++;
        count_available_mutex->unlock();

        lock.lock();
    }
}

void low_handler(
    std::mutex *mutex,
    std::condition_variable *condition_variable,
    std::deque<std::function<void(void)>> *queue,
    std::mutex *count_available_mutex,
    int *count_available,
    int thread_id
) {
    std::unique_lock<std::mutex> lock(*mutex);

    while (true) {
        // NOTE: Because we use the predicate here, if our queue only has a
        //       single item at a time (which is common in artificial test
        //       cases) one worker will become "sticky" and always handle the
        //       work.
        condition_variable->wait(lock, [queue]{
            return (queue->size());
        });

        if (queue->size() <= 0) {
            continue;
        }

        count_available_mutex->lock();
        (*count_available)--;
        count_available_mutex->unlock();

        auto a = queue->front();
        queue->pop_front();
        lock.unlock();
        a();

        count_available_mutex->lock();
        (*count_available)++;
        count_available_mutex->unlock();

        lock.lock();
    }
}

WorkQueue::WorkQueue(int dedicatedHigh, int dedicatedMediumPlus, int low) {
    this->high = new WorkQueuePriority(
        hi_handler,
        dedicatedHigh
    );
    this->medium = new WorkQueuePriority(
        medium_handler,
        dedicatedMediumPlus
    );
    this->low = new WorkQueuePriority(
        low_handler,
        low
    );
}

void WorkQueue::debug() {
    std::cout << "High: " << this->high->debug() << " Medium: " << this->medium->debug() << " Low: " << this->low->debug() << std::endl;
}

void WorkQueue::addHighWork(  std::function<void(void)> work) {
    bool foundHigh = this->high->addWork(work, false);
    if (foundHigh) return;

    bool foundMedium = this->medium->addWork(work, false);
    if (foundMedium) return;

    bool foundLow = this->low->addWork(work, false);
    if (foundLow) return;

    std::cout << "Could not find a valid queue" << std::endl;
    this->high->addWork(work, true);

    // TODO: Nothing can handle immediately. I need to put this into the first
    //       empty queue, but I cannot know what that is. So how do I handle
    //       this?
}

WorkQueue::~WorkQueue() {
    delete this->high;
    delete this->medium;
    delete this->low;
}