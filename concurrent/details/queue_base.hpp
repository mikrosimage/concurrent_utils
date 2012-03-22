/*
 * common_queue.hpp
 *
 *  Created on: Mar 22, 2012
 *      Author: Guillaume Chatelet
 */

#ifndef COMMON_QUEUE_HPP_
#define COMMON_QUEUE_HPP_

#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/call_traits.hpp>

namespace concurrent {
namespace details {

template <typename Derived, typename Container>
struct queue_base : private boost::noncopyable {
    typedef Container container_type;
    typedef typename container_type::size_type size_type;
    typedef typename container_type::value_type value_type;
    typedef typename boost::call_traits<value_type>::reference reference;
    typedef typename boost::call_traits<value_type>::param_type param_type;

    void push(param_type value) {
        boost::mutex::scoped_lock lock(m_mutex);
        exact()->wait_not_full(lock);
        exact()->_push(value);
        unlockAndNotifyNotEmpty(lock);
    }

    bool tryPush(param_type value) {
        boost::mutex::scoped_lock lock(m_mutex);
        if (exact()->is_not_full()) {
            exact()->_push(value);
            value = exact()->_pop();
            unlockAndNotifyNotEmpty(lock);
            return true;
        } else {
            return false;
        }
    }

    void pop(reference value) {
        boost::mutex::scoped_lock lock(m_mutex);
        exact()->wait_not_empty(lock);
        value = exact()->_pop();
        unlockAndNotifyNotFull(lock);
    }

    bool tryPop(reference value) {
        boost::mutex::scoped_lock lock(m_mutex);
        if (exact()->is_not_empty()) {
            value = exact()->_pop();
            unlockAndNotifyNotFull(lock);
            return true;
        } else {
            return false;
        }
    }

    void clear() {
        // locking the shared object
        boost::mutex::scoped_lock lock(m_mutex);
        if(exact()->is_not_empty()){
            exact()->_clear();
            unlockAndNotifyNotFull(lock);
        }
    }

    template<typename CompatibleContainer>
    void drainFrom(CompatibleContainer &collection) {
        if (collection.empty())
            return;
        boost::mutex::scoped_lock lock(m_mutex);
        drain<CompatibleContainer, container_type>(collection, exact()->m_container);
        unlockAndNotifyNotEmpty(lock);
    }

    template<typename CompatibleContainer>
    bool drainTo(CompatibleContainer& collection) {
        boost::mutex::scoped_lock lock(m_mutex);
        if (exact()->is_not_empty()) {
            drain<container_type, CompatibleContainer>(exact()->m_container, collection);
            unlockAndNotifyNotFull(lock);
            return true;
        }
        return false;
    }

private:
    Derived* exact() {
        return static_cast<Derived*>(this);
    }

    template<typename C1, typename C2>
    inline static void drain(C1& from, C2& to) {
        while (!from.empty()) {
            to.push_back(from.front());
            from.pop_front();
        }
    }
    void unlockAndNotifyNotFull(boost::mutex::scoped_lock &lock) {
        lock.unlock();
        exact()->notify_not_full();
    }
    void unlockAndNotifyNotEmpty(boost::mutex::scoped_lock &lock) {
        lock.unlock();
        exact()->notify_not_empty();
    }
    ::boost::mutex m_mutex;
};

}  // namespace details
}  // namespace concurrent

#endif /* COMMON_QUEUE_HPP_ */
