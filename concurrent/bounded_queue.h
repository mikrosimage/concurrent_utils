#ifndef BOUNDEDQUEUE_H_
#define BOUNDEDQUEUE_H_

#include "details/queue_base.hpp"

#include <functional>
#include <condition_variable>
#include <deque>

namespace concurrent {

/**
 * Bounded concurrent queue for safe access from several threads.
 *
 * Please note that a single Mutex is used for synchronization of front() and back()
 * thus leading to contention if consumer and producer are accessing the container at the same time.
 */
template<typename T, typename Container = std::deque<T> >
struct bounded_queue : public details::queue_base< bounded_queue<T>, Container > {
    typedef Container container_type;
    typedef typename container_type::value_type value_type;
    typedef typename container_type::size_type size_type;

    explicit bounded_queue(size_type capacity) : m_unread(0), m_container(capacity) {
    }
private:
    typedef bounded_queue<T, Container> ME;

    template<typename _T,typename _Container>
    friend struct details::queue_base;

    inline void _clear() {
        m_container.clear();
        m_unread = 0;
    }
    inline void _push(value_type value) {
        m_container.push_front(value);
        ++m_unread;
    }
    inline value_type _pop() {
        return m_container[--m_unread];
    }
    inline void wait_not_empty(std::unique_lock<std::mutex> &lock) {
        m_not_empty.wait(lock, std::bind(&ME::is_not_empty, this));
    }
    inline void wait_not_full(std::unique_lock<std::mutex> &lock) {
        m_not_full.wait(lock, std::bind(&ME::is_not_full, this));
    }
    inline bool is_not_empty() const {
        return m_unread > 0;
    }
    inline bool is_not_full() const {
        return m_unread < m_container.size();
    }
    inline void notify_not_full() {
        m_not_full.notify_one();
    }
    inline void notify_not_empty() {
        m_not_empty.notify_one();
    }
private:
    size_type m_unread;
    container_type m_container;
    std::condition_variable m_not_empty;
    std::condition_variable m_not_full;
};

} /* namespace concurrent */
#endif /* BOUNDEDQUEUE_H_ */
