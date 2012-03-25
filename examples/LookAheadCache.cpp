/*
 * ConcurrentSlot.cpp
 *
 *  Created on: Mar 21, 2012
 *      Author: Guillaume Chatelet
 */

#include <concurrent/notifier.hpp>
#include <concurrent/cache/lookahead_cache.hpp>

#include <boost/thread.hpp>

#include <sstream>
#include <iterator>
#include <algorithm>

#include <cstdio>
#include <cstdlib>

typedef int id_type;
typedef size_t metric_type;
typedef std::string data_type;

/**
 * A job is a lazy list of id_types. It is used by the cache to feed the working threads.
 *
 * It must be default constructible and have three public methods :
 * - id_type next();
 * - bool empty() const;
 * - void clear();
 *
 * This simple implementation gives numbers in the range [from, from+count[ but it
 * could be whatever you may imagine.
 */
struct Job {
    id_type from;
    size_t count;

    Job() :
                    from(0), count(0) {
    }

    Job(id_type from, size_t count) :
                    from(from), count(count) {
    }

    id_type next() {
        assert(!empty());
        --count;
        return from++;
    }

    bool empty() const {
        return count == 0;
    }

    void clear() {
        count = 0;
    }
};

// In this example each item has a cost of one, we're limiting the cache to 100 items.
const size_t max_weight = 100;
// The actual cache.
concurrent::cache::lookahead_cache<id_type, metric_type, data_type, Job> cache(max_weight);

// a simple signal to the main thread that the worker is launched and processed a least one job.
concurrent::notifier workerStarted;

/**
 * The worker function will ask work from the cache, process it and
 * put the result back in the cache.
 *
 * If the cache is terminated, every call to popWorkItem will throw a
 * concurrent::terminated exception, an easy and safe way to stop the workers
 */
void worker() {
    try {
        id_type id;
        for (;;) {
            // get some work
            cache.pop(id);
            // process
            std::ostringstream str;
            str << "data with value " << id;
            // push back to cache
            cache.push(id, 1, str.str());
            // notifying main thread we did at least one element
            workerStarted.ack();
        }
    } catch (concurrent::terminated &e) {
        printf("worker : terminates\n");
    }
}

void check(id_type id) {
    std::string result;
    printf("main : cache has value %d ? ", id);
    const bool success = cache.get(id, result);
    if (success)
        printf("yes, was '%s'\n", result.c_str());
    else
        printf("no\n");
}

int main(int argc, char **argv) {
    // For the sake of simplicity we're starting only one thread here
    // but you can add as many workers as you want.
    boost::thread worker_thread(&::worker);

    // posting a first job
    cache.process(Job(1, 10));

    // waiting for the worker to start
    workerStarted.wait();

    // checking if some data is in the cache
    check(1005);
    check(1);
    check(9);

    // posting another job
    cache.process(Job(1000, 20));

    // checking some more data
    check(2);
    check(5);
    check(1000);

    // requesting termination
    printf("main : sending termination\n");
    cache.terminate();

    // waiting for the thread to finish
    worker_thread.join();

    // as a bonus we can see what was actually processed by the worker
    std::vector<id_type> keys;
    printf("main : worker processed %lu elements : ", cache.dumpKeys(keys));
    std::copy(keys.begin(), keys.end(), std::ostream_iterator<id_type>(std::cout, " "));
    std::cout << std::endl;

    return EXIT_SUCCESS;
}
