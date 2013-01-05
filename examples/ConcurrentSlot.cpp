/*
 * ConcurrentSlot.cpp
 *
 *  Created on: Mar 21, 2012
 *      Author: Guillaume Chatelet
 */

#include <concurrent/slot.hpp>
#include <concurrent/notifier.hpp>

#include <thread>

#include <cstdio>
#include <cstdlib>

static concurrent::slot<int> input; ///< shared
static concurrent::notifier notifier; ///< shared

static void worker() {
    try {
        for (;;) {
            int value;
            input.waitGet(value);
            printf("worker got : %d\n", value);
            notifier.ack();
        }
    } catch (concurrent::terminated &e) {
        printf("worker terminates\n");
    }
}

int main(int argc, char **argv) {
    std::thread worker_thread(&::worker);

    for (int i = 0; i < 10; ++i) {
        printf("main sending : %d\n", i);
        input.set(i);
        notifier.wait();
    }

    printf("main sending termination\n");
    input.terminate();

    worker_thread.join();

    return EXIT_SUCCESS;
}
