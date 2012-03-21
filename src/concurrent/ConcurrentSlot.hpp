/*
 * ConcurrentSlot.hpp
 *
 *  Created on: Mar 21, 2012
 *      Author: Guillaume Chatelet
 */

#ifndef CONCURRENTSLOT_HPP_
#define CONCURRENTSLOT_HPP_

#include "Common.hpp"

#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/condition_variable.hpp>

#include <cassert>

namespace concurrent {

/**
 * Thread safe access to a T object
 *
 * By setting terminate to true, getters/setters will throw a terminated exception
 */
template<typename T>
struct ConcurrentSlot : private boost::noncopyable {
    ConcurrentSlot() : m_SharedObjectSet(false), m_SharedTerminate(false) {
    }

    ConcurrentSlot(const T&object) : m_SharedObject(object), m_SharedObjectSet(true), m_SharedTerminate(false) {
    }

    void set(const T& object) {
        // locking the shared object
        ::boost::mutex::scoped_lock lock(m_Mutex);

        checkTermination();

        internal_set(object);
        lock.unlock();
        // notifying shared structure is updated
        m_Condition.notify_one();
    }

    void terminate(bool value = true) {
        ::boost::unique_lock<boost::mutex> lock(m_Mutex);
        m_SharedTerminate = value;
        lock.unlock();
        m_Condition.notify_all();
    }

    void waitGet(T& value) {
        ::boost::unique_lock<boost::mutex> lock(m_Mutex);

        checkTermination();

        // blocking until set or terminate
        while (!m_SharedObjectSet) {
            m_Condition.wait(lock);
            checkTermination();
        }

        internal_unset(value);
    }

    bool tryGet(T& holder) {
        // locking the shared object
        ::boost::lock_guard<boost::mutex> lock(m_Mutex);
        checkTermination();

        if (!m_SharedObjectSet)
            return false;

        internal_unset(holder);
        return true;
    }
private:
    inline void checkTermination() const {
        // mutex *must* be locked here, we are reading a shared variable
        if (m_SharedTerminate)
            throw terminated();
    }

    inline void internal_set(const T& value){
        m_SharedObject = value;
        m_SharedObjectSet = true;
    }

    inline void internal_unset(T& value){
        assert(m_SharedObjectSet);
        value = m_SharedObject;
        m_SharedObjectSet = false;
    }

    mutable ::boost::mutex m_Mutex;
    ::boost::condition_variable m_Condition;
    T m_SharedObject;
    bool m_SharedObjectSet;
    bool m_SharedTerminate;
};

}  // namespace concurrent

#endif /* CONCURRENTSLOT_HPP_ */
