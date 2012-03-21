#ifndef BOUNDEDQUEUE_H_
#define BOUNDEDQUEUE_H_

#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/call_traits.hpp>
#include <boost/bind.hpp>

#include <boost/circular_buffer.hpp>

namespace concurrent {

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
        m_not_full.wait(lock, boost::bind(&bounded_queue<value_type>::is_not_full, this));
        _push(value);
        lock.unlock();
        m_not_empty.notify_one();
    }

    bool tryPush(param_type value) {
        boost::mutex::scoped_lock lock(m_mutex);
        if (is_not_full()) {
            _push(value);
            value = _pop();
            lock.unlock();
            m_not_empty.notify_one();
            return true;
        } else {
            return false;
        }
    }

    void waitPop(reference value) {
        boost::mutex::scoped_lock lock(m_mutex);
        m_not_empty.wait(lock, boost::bind(&bounded_queue<value_type>::is_not_empty, this));
        value = _pop();
        lock.unlock();
        m_not_full.notify_one();
    }

    bool tryPop(reference value) {
        boost::mutex::scoped_lock lock(m_mutex);
        if (is_not_empty()) {
            value = _pop();
            lock.unlock();
            m_not_full.notify_one();
            return true;
        } else {
            return false;
        }
    }

    void clear() {
        // locking the shared object
        boost::mutex::scoped_lock lock(m_mutex);
        m_container.clear();
        m_unread = 0;
    }
private:
    inline void _push(value_type value) {
        m_container.push_front(value);
        ++m_unread;
    }
    inline value_type _pop() {
        return m_container[--m_unread];
    }

    inline bool is_not_empty() const {
        return m_unread > 0;
    }
    inline bool is_not_full() const {
        return m_unread < m_container.capacity();
    }

    size_type m_unread;
    container_type m_container;
    ::boost::mutex m_mutex;
    ::boost::condition m_not_empty;
    ::boost::condition m_not_full;
};

} /* namespace concurrent */
#endif /* BOUNDEDQUEUE_H_ */
