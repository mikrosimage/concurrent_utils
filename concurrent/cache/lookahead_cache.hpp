/*
 * LookAheadCache.hpp
 *
 *  Created on: 22 nov. 2011
 *      Author: Guillaume Chatelet
 */

#ifndef LOOK_AHEAD_CACHE_HPP_
#define LOOK_AHEAD_CACHE_HPP_

#include "priority_cache.hpp"

#include <concurrent/slot.hpp>

#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>

#include <iostream>
#include <map>
#include <set>
#include <cassert>

namespace concurrent {

namespace cache {

/**
 *
 */
template<typename ID_TYPE, typename METRIC_TYPE, typename DATA_TYPE, typename WORK_UNIT_RANGE>
struct lookahead_cache {
    typedef ID_TYPE id_type;
    typedef METRIC_TYPE metric_type;
    typedef DATA_TYPE data_type;
    typedef WORK_UNIT_RANGE WorkUnitItr;

    BOOST_CONCEPT_ASSERT((boost::DefaultConstructible<WORK_UNIT_RANGE>));


    lookahead_cache(const metric_type cache_limit) :
        m_SharedCache(cache_limit) {
    }

    // Cache functions
    inline bool get(const id_type &id, data_type &data) const {
        boost::mutex::scoped_lock lock(m_CacheMutex);
        return m_SharedCache.get(id, data);
    }

    inline metric_type dumpKeys(std::vector<id_type> &allKeys) const {
        boost::mutex::scoped_lock lock(m_CacheMutex);
        m_SharedCache.dumpKeys(allKeys);
        return m_SharedCache.weight();
    }

    void pushNewJob(const WorkUnitItr &job) {
        m_PendingJob.set(job);
    }

    inline void setCacheSize(const metric_type size) {
        boost::mutex::scoped_lock lock(m_CacheMutex);
        m_SharedCache.setCacheSize(size);
    }
    void terminate(bool value = true) {
        m_PendingJob.terminate(value);
    }

    // worker functions
    void popWorkItem(id_type &unit) {
        boost::mutex::scoped_lock lock(m_WorkerMutex);
        do {
            unit = nextWorkUnit();
            D_( std::cout << "next unit is : " << unit.filename << std::endl);
            boost::mutex::scoped_lock lock(m_CacheMutex);
            switch (m_SharedCache.update(unit)) {
                case FULL:
                    D_( std::cout << "cache is full, emptying current job" << std::endl);
                    m_SharedWorkUnitItr.clear();
                    break;
                case NOT_NEEDED:
                    D_( std::cout << "unit updated, checking another one" << std::endl);
                    break;
                case NEEDED:
                    D_( std::cout << "serving " << unit << std::endl);
                    return;
            }
        } while (true);
    }

    inline bool putWorkItem(const id_type &id, const metric_type weight, const data_type &data) {
        boost::mutex::scoped_lock lock(m_CacheMutex);
        return m_SharedCache.put(id, weight, data);
    }

private:
    inline id_type nextWorkUnit() {
        if (updateJob()) {
            boost::mutex::scoped_lock lock(m_CacheMutex);
            m_SharedCache.discardPending();
        }
        return m_SharedWorkUnitItr.next();
    }

    inline bool updateJob() {
        bool updated = m_PendingJob.tryGet(m_SharedWorkUnitItr);
        while (m_SharedWorkUnitItr.empty()) {
            m_PendingJob.waitGet(m_SharedWorkUnitItr);
            updated = true;
        }
        assert(!m_SharedWorkUnitItr.empty());
        return updated;
    }

    boost::mutex m_WorkerMutex;
    mutable boost::mutex m_CacheMutex;
    priority_cache<id_type, metric_type, data_type> m_SharedCache;
    slot<WorkUnitItr> m_PendingJob;
    WorkUnitItr m_SharedWorkUnitItr;
};

} // namespace cache

}  // namespace concurrent

#endif /* LOOK_AHEAD_CACHE_HPP_ */
