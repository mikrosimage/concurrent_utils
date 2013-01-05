/*
 * ConcurrentQueue.cpp
 *
 *  Created on: Mar 21, 2012
 *      Author: Guillaume Chatelet
 */

#include <concurrent/queue.hpp>

#include <thread>

#include <cstdlib>
#include <cstdio>

static concurrent::queue<int> g_queue; ///< shared

static void worker() {
    int value = 0;
    while (g_queue.tryPop(value))
        printf("worker popped %d\n", value);
}

int main(int argc, char **argv) {
    for (int i = 0; i < 10; ++i)
        g_queue.push(i);

    std::thread worker_thread(&worker);

    worker_thread.join();
    return EXIT_SUCCESS;
}
