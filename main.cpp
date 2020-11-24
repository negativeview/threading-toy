#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>

#include "src/WorkQueue.hpp"

void foo2() {
    std::cout << "foo2" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));
}

int main() {
    WorkQueue *workQueue = new WorkQueue(5, 5, 10);

    workQueue->addHighWork(foo2);

    delete workQueue;

    return 0;
}