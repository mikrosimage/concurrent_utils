#ifndef BOUNDEDQUEUE_H_
#define BOUNDEDQUEUE_H_

#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/call_traits.hpp>
#include <boost/bind.hpp>

#include <boost/circular_buffer.hpp>

namespace concurrent {

/**
 * Bounded concurrent queue for safe access from several threads.
 *
 * Please note that a single Mutex is used for synchronization of front() and back()
 * thus leading to contention if consumer and producer are accessing the container at the same time.
 */
template<class T>
struct bounded_queue : private boost::noncopyable {
    typedef boost::circular_buffer<T> container_type;
    typedef typename container_type::size_type size_type;
    typedef typename container_type::value_type value_type;
    typedef typename boost::call_traits<value_type>::reference reference;
    typedef typename boost::call_traits<value_type>::param_type param_type;

    explicit bounded_queue(size_type capacity) :
                    m_unread(0), m_container(capacity) {
    }

    void waitPush(param_type value) {
        boost::mutex::scoped_lock lock(m_mutex);
        wait_not_full(lock);
        _push(value);
        unlockAndNotifyNotEmpty(lock);
    }

    bool tryPush(param_type value) {
        boost::mutex::scoped_lock lock(m_mutex);
        if (is_not_full()) {
            _push(value);
            value = _pop();
            unlockAndNotifyNotEmpty(lock);
            return true;
        } else {
            return false;
        }
    }

    void waitPop(reference value) {
        boost::mutex::scoped_lock lock(m_mutex);
        wait_not_empty(lock);
        value = _pop();
        unlockAndNotifyNotFull(lock);
    }

    bool tryPop(reference value) {
        boost::mutex::scoped_lock lock(m_mutex);
        if (is_not_empty()) {
            value = _pop();
            unlockAndNotifyNotFull(lock);
            return true;
        } else {
            return false;
        }
    }

    void clear() {
        // locking the shared object
        boost::mutex::scoped_lock lock(m_mutex);
        if(is_not_empty()){
            _clear();
            unlockAndNotifyNotFull(lock);
        }
    }

    template<typename CompatibleContainer>
    void drainFrom(CompatibleContainer &collection) {
        if (collection.empty())
            return;
        boost::mutex::scoped_lock lock(m_mutex);
        drain<CompatibleContainer, container_type>(collection, m_container);
        unlockAndNotifyNotEmpty(lock);
    }

    template<typename CompatibleContainer>
    bool drainTo(CompatibleContainer& collection) {
        if (m_container.empty())
            return false;
        boost::mutex::scoped_lock lock(m_mutex);
        drain<container_type, CompatibleContainer>(m_container, collection);
        unlockAndNotifyNotFull(lock);
        return true;
    }

private:
    template<typename C1, typename C2>
    inline static void drain(C1& from, C2& to) {
        while (!from.empty()) {
            to.push_back(from.front());
            from.pop_front();
        }
    }
    void unlockAndNotifyNotFull(boost::mutex::scoped_lock &lock) {
        lock.unlock();
        notify_not_full();
    }
    void unlockAndNotifyNotEmpty(boost::mutex::scoped_lock &lock) {
        lock.unlock();
        notify_not_empty();
    }
    ::boost::mutex m_mutex;
private:
    // container specific
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
    inline void wait_not_empty(boost::mutex::scoped_lock &lock) {
        m_not_empty.wait(lock, boost::bind(&bounded_queue<value_type>::is_not_empty, this));
    }
    inline void wait_not_full(boost::mutex::scoped_lock &lock) {
        m_not_full.wait(lock, boost::bind(&bounded_queue<value_type>::is_not_full, this));
    }
    inline bool is_not_empty() const {
        return m_unread > 0;
    }
    inline bool is_not_full() const {
        return m_unread < m_container.capacity();
    }
    inline void notify_not_full() {
        m_not_full.notify_one();
    }
    inline void notify_not_empty() {
        m_not_empty.notify_one();
    }
    size_type m_unread;
    container_type m_container;
    ::boost::condition m_not_empty;
    ::boost::condition m_not_full;
};

} /* namespace concurrent */
#endif /* BOUNDEDQUEUE_H_ */
