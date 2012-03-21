#ifndef CONCURRENTQUEUE_H_
#define CONCURRENTQUEUE_H_

#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/call_traits.hpp>
#include <boost/bind.hpp>

#include <deque>

namespace concurrent {

/**
 * Unbounded concurrent queue for safe access from several threads.
 *
 * Please note that a single Mutex is used for synchronization of front() and back()
 * thus leading to contention if consumer and producer are accessing the container at the same time.
 */
template<typename T, typename Container = std::deque<T> >
struct queue : private boost::noncopyable {
public:
    typedef Container container_type;
    typedef typename Container::value_type value_type;
    typedef typename Container::const_reference const_reference;
    typedef typename boost::call_traits<value_type>::reference reference;
    typedef typename boost::call_traits<value_type>::param_type param_type;

    void push(param_type value) {
        boost::mutex::scoped_lock lock(m_mutex);
        _push(value);
        lock.unlock();
        m_not_empty.notify_one();
    }

    template<typename CompatibleContainer>
    void drainFrom(CompatibleContainer &collection) {
        if (collection.empty())
            return;
        boost::mutex::scoped_lock lock(m_mutex);
        drain<CompatibleContainer, Container>(collection, m_container);
        m_not_empty.notify_one();
    }

    void waitPop(reference value) {
        boost::mutex::scoped_lock lock(m_mutex);
        m_not_empty.wait(lock, boost::bind(&queue<value_type>::is_not_empty, this));
        value = _pop();
    }

    bool tryPop(reference value) {
        boost::mutex::scoped_lock lock(m_mutex);
        if (is_not_empty()) {
            value = _pop();
            return true;
        } else {
            return false;
        }
    }

    template<typename CompatibleContainer>
    bool drainTo(CompatibleContainer& collection) {
        if (m_container.empty())
            return false;
        drain<Container, CompatibleContainer>(m_container, collection);
        return true;
    }

    void clear() {
        boost::mutex::scoped_lock lock(m_mutex);
        m_container.clear();
    }
private:
    inline void _push(value_type value) {
        m_container.push_back(value);
    }
    inline value_type _pop() {
        value_type tmp = m_container.front();
        m_container.pop_front();
        return tmp;
    }
    inline bool is_not_empty() const {
        return !m_container.empty();
    }
    inline bool is_not_full() const {
        return true;
    }

    template<typename C1, typename C2>
    inline static void drain(C1& from, C2& to) {
        while (!from.empty()) {
            to.push_back(from.front());
            from.pop_front();
        }
    }

    container_type m_container;
    ::boost::mutex m_mutex;
    ::boost::condition_variable m_not_empty;
};

} // namespace concurrent

#endif /* CONCURRENTQUEUE_H_ */
