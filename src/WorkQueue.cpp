#include "WorkQueue.hpp"

#include <iostream>

/**
 * TODO: The three handlers can share some code. I am not consolidating them
 *       YET because I need to work out how to make the queues do priorities
 *       correctly. Once that's done I can start consolidating.
 */
void hi_handler(
    std::mutex *mutex,
    std::condition_variable *condition_variable,
    std::deque<std::function<void(void)>> *queue,
    int thread_id
) {
    std::cout << "Starting high thread " << thread_id << std::endl;

    std::unique_lock<std::mutex> lock(*mutex);

    while (true) {
        // NOTE: Because we use the predicate here, if our queue only has a
        //       single item at a time (which is common in artificial test
        //       cases) one worker will become "sticky" and always handle the
        //       work.
        condition_variable->wait(lock, [queue]{
            return (queue->size());
        });
        // wait(lock);

        if (queue->size() <= 0) {
            continue;
        }

        auto a = queue->front();
        queue->pop_front();
        lock.unlock();
        a();

        lock.lock();
    }
}

void medium_handler(
    std::mutex *mutex,
    std::condition_variable *condition_variable,
    std::deque<std::function<void(void)>> *queue,
    int thread_id
) {
    std::cout << "Starting medium thread " << thread_id << std::endl;

    std::unique_lock<std::mutex> lock(*mutex);

    while (true) {
        // NOTE: Because we use the predicate here, if our queue only has a
        //       single item at a time (which is common in artificial test
        //       cases) one worker will become "sticky" and always handle the
        //       work.
        condition_variable->wait(lock, [queue]{
            return (queue->size());
        });
        // wait(lock);

        if (queue->size() <= 0) {
            continue;
        }

        auto a = queue->front();
        queue->pop_front();
        lock.unlock();
        a();

        lock.lock();
    }
}

void low_handler(
    std::mutex *mutex,
    std::condition_variable *condition_variable,
    std::deque<std::function<void(void)>> *queue,
    int thread_id
) {
    std::cout << "Starting low thread " << thread_id << std::endl;

    std::unique_lock<std::mutex> lock(*mutex);

    while (true) {
        // NOTE: Because we use the predicate here, if our queue only has a
        //       single item at a time (which is common in artificial test
        //       cases) one worker will become "sticky" and always handle the
        //       work.
        condition_variable->wait(lock, [queue]{
            return (queue->size());
        });
        // wait(lock);

        if (queue->size() <= 0) {
            continue;
        }

        auto a = queue->front();
        queue->pop_front();
        lock.unlock();
        a();

        lock.lock();
    }
}

WorkQueue::WorkQueue(int dedicatedHigh, int dedicatedMediumPlus, int low) {
    this->high_queue = new std::deque<std::function<void(void)>>();
    this->medium_queue = new std::deque<std::function<void(void)>>();
    this->low_queue = new std::deque<std::function<void(void)>>();

    for (int i = 0; i < dedicatedHigh; i++) {
        this->hi_threads.emplace_back(
            new std::thread(
                std::bind(
                    hi_handler,
                    &this->high_mutex,
                    &this->high_condition,
                    this->high_queue,
                    i
                )
            )
        );
    }

    for (int i = 0; i < dedicatedMediumPlus; i++) {
        this->medium_threads.emplace_back(
            new std::thread(
                std::bind(
                    medium_handler,
                    &this->medium_mutex,
                    &this->medium_condition,
                    this->medium_queue,
                    i
                )
            )
        );
    }

    for (int i = 0; i < low; i++) {
        this->low_threads.emplace_back(
            new std::thread(
                std::bind(
                    low_handler,
                    &this->low_mutex,
                    &this->low_condition,
                    this->low_queue,
                    i
                )
            )
        );
    }
}

void WorkQueue::addHighWork(  std::function<void(void)> work) {
    std::unique_lock<std::mutex> lock(this->high_mutex);

    this->high_queue->emplace_back(work);
    lock.unlock();

    // Wake all the queues, so that lower priority threads can handle higher
    // priority things. This does mean that we have a lot of spurious wake
    // ups because we cannot easily tell if a higher priority thread will
    // handle it.

    // We might be able to find a better way to do this, but this "works" for
    // now, it just sucks that we're putting in known pathological cases when
    // the entire point of a work queue is optimization.
    this->high_condition.notify_one();
    this->medium_condition.notify_one();
    this->low_condition.notify_one();
}

void WorkQueue::addMediumWork(  std::function<void(void)> work) {
    std::unique_lock<std::mutex> lock(this->medium_mutex);

    this->medium_queue->emplace_back(work);
    lock.unlock();

    // Wake all the queues, so that lower priority threads can handle higher
    // priority things. This does mean that we have a lot of spurious wake
    // ups because we cannot easily tell if a higher priority thread will
    // handle it.

    // We might be able to find a better way to do this, but this "works" for
    // now, it just sucks that we're putting in known pathological cases when
    // the entire point of a work queue is optimization.
    this->medium_condition.notify_one();
    this->low_condition.notify_one();
}

void WorkQueue::addLowWork(  std::function<void(void)> work) {
    std::unique_lock<std::mutex> lock(this->low_mutex);

    this->low_queue->emplace_back(work);
    lock.unlock();

    this->low_condition.notify_one();
}

WorkQueue::~WorkQueue() {
    for (auto &thread : this->hi_threads) {
        thread->join();
        delete thread;
    }

    for (auto &thread : this->medium_threads) {
        thread->join();
        delete thread;
    }

    for (auto &thread : this->low_threads) {
        thread->join();
        delete thread;
    }
}