/*
 * ConcurrentSlot.cpp
 *
 *  Created on: Mar 21, 2012
 *      Author: Guillaume Chatelet
 */

#include <concurrent/ConcurrentSlot.hpp>

#include <boost/thread.hpp>

#include <cstdio>
#include <cstdlib>

concurrent::ConcurrentSlot<int> input; ///< shared
concurrent::ConcurrentSlot<bool> ack; ///< shared

void worker() {
    try {
        for (;;) {
            int value;
            input.waitGet(value);
            printf("worker got : %d\n", value);
            ack.set(true);
        }
    } catch (concurrent::terminated &e) {
        printf("worker terminates\n");
    }
}

int main(int argc, char **argv) {
    boost::thread worker_thread(&::worker);

    bool dummy;
    for (int i = 0; i < 10; ++i) {
        printf("main sending : %d\n", i);
        input.set(i);
        ack.waitGet(dummy);
    }

    printf("main sending termination\n");
    input.terminate();

    worker_thread.join();

    return EXIT_SUCCESS;
}
