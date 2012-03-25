/*
 * ConcurrentQueue.cpp
 *
 *  Created on: Mar 21, 2012
 *      Author: Guillaume Chatelet
 */

#include <concurrent/bounded_queue.h>

#include <boost/thread.hpp>

#include <cstdlib>
#include <cstdio>


const int sentinel_value = -1;
const size_t queue_max_element = 3;
concurrent::bounded_queue<int> g_queue(queue_max_element); ///< shared

void worker() {
    int value = 0;
    for (;;) {
        g_queue.pop(value);
        if (value == sentinel_value)
            return;
        printf("worker popped %d\n", value);
    }
}

int main(int argc, char **argv) {
    boost::thread worker_thread(&worker);

    for (int i = 0; i < 10; ++i) {
        g_queue.push(i);
        printf("main thread pushed %d\n", i);
    }

    g_queue.push(sentinel_value);

    worker_thread.join();
    return EXIT_SUCCESS;
}
