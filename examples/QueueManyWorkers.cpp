/*
 * ConcurrentQueue.cpp
 *
 *  Created on: Mar 21, 2012
 *      Author: Guillaume Chatelet
 */

#include <concurrent/queue.hpp>

#include <boost/thread.hpp>

#include <sstream>
#include <cstdlib>
#include <cstdio>

concurrent::queue<int> g_queue; ///< shared

void worker() {
    int value = 0;
    while (g_queue.tryPop(value)){
        std::ostringstream line;
        line << "worker " <<  boost::this_thread::get_id() << " popped " << value;
        printf("%s\n", line.str().c_str());
    }
}

int main(int argc, char **argv) {
    for (int i = 0; i < 20; ++i)
        g_queue.push(i);

    boost::thread_group thread_group;

    for (size_t i = 0; i < 3; ++i)
        thread_group.create_thread(&worker);

    thread_group.join_all();
    return EXIT_SUCCESS;
}
