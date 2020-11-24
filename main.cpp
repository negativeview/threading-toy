#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>

#include "src/WorkQueue.hpp"

void foo2() {
    std::this_thread::sleep_for(std::chrono::seconds(10));
}

int main() {
    WorkQueue *workQueue = new WorkQueue(5, 5, 10);

    for (int i = 0; i < 25; i++) {
        workQueue->addHighWork(foo2);
    }

    while (true) {
        workQueue->debug();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    delete workQueue;

    return 0;
}