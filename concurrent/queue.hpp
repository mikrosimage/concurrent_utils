#ifndef CONCURRENTQUEUE_H_
#define CONCURRENTQUEUE_H_

#include "details/queue_base.hpp"

#include <condition_variable>
#include <functional>
#include <deque>

namespace concurrent {

/**
 * Unbounded concurrent queue for safe access from several threads.
 *
 * Please note that a single Mutex is used for synchronization of front() and back()
 * thus leading to contention if consumer and producer are accessing the container at the same time.
 */
template<typename T, typename Container = std::deque<T> >
struct queue : public details::queue_base< queue<T>, Container > {
    typedef Container container_type;
    typedef typename Container::value_type value_type;
    typedef typename Container::const_reference const_reference;

private:
    typedef queue<T, Container> ME;

    template<typename _T,typename _Container>
    friend struct details::queue_base;

    inline void _clear() {
        m_container.clear();
    }
    inline void _push(value_type value) {
        m_container.push_back(value);
    }
    inline value_type _pop() {
        value_type tmp(m_container.front());
        m_container.pop_front();
        return tmp;
    }
    inline void wait_not_empty(std::unique_lock<std::mutex> &lock) {
        m_not_empty.wait(lock, std::bind(&ME::is_not_empty, this));
    }
    inline void wait_not_full(std::unique_lock<std::mutex> &lock) {
    }
    inline bool is_not_empty() const {
        return !m_container.empty();
    }
    inline bool is_not_full() const {
        return true;
    }
    inline void notify_not_full() {
    }
    inline void notify_not_empty() {
        m_not_empty.notify_one();
    }
private:
    container_type m_container;
    std::condition_variable m_not_empty;
};

} // namespace concurrent

#endif /* CONCURRENTQUEUE_H_ */
