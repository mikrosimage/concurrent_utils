/*
 * ConcurrentSlot.cpp
 *
 *  Created on: Mar 21, 2012
 *      Author: Guillaume Chatelet
 */

#include <concurrent/slot.hpp>
#include <concurrent/response.hpp>

#include <boost/thread.hpp>

#include <cstdio>
#include <cstdlib>

concurrent::slot<int> input; ///< shared
concurrent::response response; ///< shared

void worker() {
    try {
        for (;;) {
            int value;
            input.waitGet(value);
            printf("worker got : %d\n", value);
            response.ack();
        }
    } catch (concurrent::terminated &e) {
        printf("worker terminates\n");
    }
}

int main(int argc, char **argv) {
    boost::thread worker_thread(&::worker);

    for (int i = 0; i < 10; ++i) {
        printf("main sending : %d\n", i);
        input.set(i);
        response.wait();
    }

    printf("main sending termination\n");
    input.terminate();

    worker_thread.join();

    return EXIT_SUCCESS;
}
