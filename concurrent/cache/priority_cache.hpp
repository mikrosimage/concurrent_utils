/*
 * priority_cache.hpp
 *
 *  Created on: 22 nov. 2011
 *      Author: Guillaume Chatelet
 */

#ifndef PRIORITY_CACHE_HPP_
#define PRIORITY_CACHE_HPP_

#include "priority_cache_details.hpp"

#include <iostream>
#include <cassert>

namespace concurrent {

namespace cache {

/**
 * Unsynchronized cache
 * Basic usage :
 * - create cache
 * - add an iterator to process
 * - loop on pop until false, for each unit process and push to cache
 */
template<typename ID_TYPE, typename METRIC_TYPE, typename DATA_TYPE, typename WORK_UNIT_RANGE>
struct priority_cache {
    typedef ID_TYPE id_type;
    typedef METRIC_TYPE metric_type;
    typedef DATA_TYPE data_type;
    typedef WORK_UNIT_RANGE WorkUnitItr;

#if __cplusplus >= 201103L
    static_assert(std::is_default_constructible<WORK_UNIT_RANGE>::value, "WorkUnitItr should be default constructible");
#endif

    priority_cache(const metric_type cache_limit) :
        m_Cache(cache_limit) {
    }

    // Cache functions
    inline bool get(const id_type &id, data_type &data) const {
        return m_Cache.get(id, data);
    }

    inline metric_type dumpKeys(std::vector<id_type> &allKeys) const {
        m_Cache.dumpKeys(allKeys);
        return m_Cache.weight();
    }

    void process(const WorkUnitItr &job) {
    	m_WorkUnitItr = job;
    }

    inline void setMaxWeight(const metric_type size) {
        m_Cache.setMaxWeight(size);
    }

    // worker functions
    bool pop(id_type &unit) {
        do {
        	if(m_WorkUnitItr.empty())
        		return false;
            unit = m_WorkUnitItr.next();
            D_( std::cout << "next unit is : " << unit.filename << std::endl);
            switch (m_Cache.update(unit)) {
                case FULL:
                    D_( std::cout << "cache is full, emptying current job" << std::endl);
                    m_WorkUnitItr.clear();
                    break;
                case NOT_NEEDED:
                    D_( std::cout << "unit updated, checking another one" << std::endl);
                    break;
                case NEEDED:
                    D_( std::cout << "serving " << unit << std::endl);
                    return true;
            }
        } while (true);
    }

    inline bool push(const id_type &id, const metric_type weight, const data_type &data) {
        return m_Cache.put(id, weight, data);
    }

private:
    priority_cache_details<id_type, metric_type, data_type> m_Cache;
    WorkUnitItr m_WorkUnitItr;
};

} // namespace cache

}  // namespace concurrent

#endif /* PRIORITY_CACHE_HPP_ */
