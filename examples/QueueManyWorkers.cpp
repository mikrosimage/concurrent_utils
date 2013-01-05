/*
 * ConcurrentQueue.cpp
 *
 *  Created on: Mar 21, 2012
 *      Author: Guillaume Chatelet
 */

#include <concurrent/queue.hpp>

#include <thread>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <cstdio>

static concurrent::queue<int> g_queue; ///< shared

static void worker() {
	int value = 0;
	while (g_queue.tryPop(value)) {
		std::ostringstream line;
		line << "worker " << std::this_thread::get_id() << " popped " << value;
		printf("%s\n", line.str().c_str());
	}
}

int main(int argc, char **argv) {
	for (int i = 0; i < 20; ++i)
		g_queue.push(i);

	std::vector<std::thread> thread_group;

	for (size_t i = 0; i < 3; ++i)
		thread_group.emplace_back(&worker);

	for (std::thread &thread : thread_group)
		thread.join();

	return EXIT_SUCCESS;
}
