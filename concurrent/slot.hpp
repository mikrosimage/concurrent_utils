/*
 * ConcurrentSlot.hpp
 *
 *  Created on: Mar 21, 2012
 *      Author: Guillaume Chatelet
 */

#ifndef CONCURRENTSLOT_HPP_
#define CONCURRENTSLOT_HPP_

#include "common.hpp"

#include <mutex>
#include <condition_variable>
#include <cassert>

namespace concurrent {

/**
 * Thread safe access to a T object
 *
 * By setting terminate to true, getters will throw a terminated exception
 */
template<typename T>
struct slot : private noncopyable {
    slot() : m_SharedObjectSet(false), m_SharedTerminate(false) {
    }

    slot(const T&object) : m_SharedObject(object), m_SharedObjectSet(true), m_SharedTerminate(false) {
    }

    void set(const T& object) {
        // locking the shared object
        std::unique_lock<std::mutex> lock(m_Mutex);
        internal_set(object);
        lock.unlock();
        // notifying shared structure is updated
        m_Condition.notify_one();
    }

    void terminate(bool value = true) {
    	std::unique_lock<std::mutex> lock(m_Mutex);
        m_SharedTerminate = value;
        lock.unlock();
        m_Condition.notify_all();
    }

    void waitGet(T& value) {
    	std::unique_lock<std::mutex> lock(m_Mutex);

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
        ::std::lock_guard<std::mutex> lock(m_Mutex);
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

    mutable std::mutex m_Mutex;
    ::std::condition_variable m_Condition;
    T m_SharedObject;
    bool m_SharedObjectSet;
    bool m_SharedTerminate;
};

}  // namespace concurrent

#endif /* CONCURRENTSLOT_HPP_ */
