/*
 * PriorityCache.hpp
 *
 *  Created on: 22 nov. 2011
 *      Author: Guillaume Chatelet
 */

#ifndef PRIORITYCACHE_DETAILS_HPP_
#define PRIORITYCACHE_DETAILS_HPP_

#include <concurrent/common.hpp>

#include <vector>
#include <deque>
#include <map>
#include <algorithm>
#include <stdexcept>
#include <type_traits>
#include <cassert>

//#define DEBUG_CACHE

#ifdef DEBUG_CACHE
#include <iostream>
#define D_(X) (X)
#else
#define D_(X)
#endif

namespace concurrent {
namespace cache {

enum UpdateStatus {
	FULL, NEEDED, NOT_NEEDED
};

/**
 * This is the backend for the look ahead cache. It is thread unsafe and
 * not meant to be used directly.
 */
template<typename ID_TYPE, typename METRIC_TYPE, typename DATA_TYPE>
struct priority_cache_details: private noncopyable {
	typedef ID_TYPE id_type;
	typedef METRIC_TYPE metric_type;
	typedef DATA_TYPE data_type;

	static_assert(std::is_unsigned<metric_type>::value, "metric_type must be unsigned");

private:
	typedef std::deque<id_type> IdContainer;
	typedef typename IdContainer::iterator IdItr;

	struct WeightedData {
		metric_type weight;
		data_type data;
		WeightedData(const metric_type &weight, const data_type &data) :
				weight(weight), data(data) {
		}
	};

	typedef std::map<id_type, WeightedData> CacheContainer;
	typedef typename CacheContainer::const_iterator CacheConstItr;
	typedef typename CacheContainer::iterator CacheItr;

public:
	priority_cache_details(metric_type limit) :
			m_MaxWeight(limit) {
		D_( std::cout << "########################################" << std::endl);
	}

	void dumpKeys(std::vector<id_type> &key_container) const {
		key_container.clear();
		key_container.reserve(m_Cache.size());
		for (const auto& pair : m_Cache)
			key_container.push_back(pair.first);
	}

	inline bool full() const {
		return m_MaxWeight == 0 || contiguousWeight() > m_MaxWeight;
	}

	inline bool contains(id_type id) const {
		return m_Cache.find(id) != m_Cache.end();
	}

	inline bool pending(id_type id) const {
		return in(m_PendingIds, id);
	}

	inline metric_type weight() const {
		return currentWeight();
	}

	void discardPending() {
		m_DiscardableIds.insert(m_DiscardableIds.begin(), m_PendingIds.begin(), m_PendingIds.end());
		m_PendingIds.clear();
	}

	UpdateStatus update(id_type id) {
		if (full())
			return FULL; //
		D_( std::cout << "Updating " << id << std::endl);
		const bool wasRequested = remove(id);
		m_PendingIds.push_back(id);
		const UpdateStatus status = wasRequested || contains(id) ? NOT_NEEDED : NEEDED;
		if (status == NEEDED)
			dump("update dump");
		return status;
	}

	bool put(const id_type &id, const metric_type weight, const data_type &data) {
		D_( std::cout << "========================================" << std::endl);
		if (weight == 0)
			throw std::logic_error("can't put an id with no weight");
		if (contains(id))
			throw std::logic_error("id is already present in cache");

		if (full()) {
			D_( std::cout << "cache is *full*, discarding " << id << std::endl);
			remove(id); // no more pending
			dump("cache full dump");
			return false;
		}
		if (!canFit(weight)) {
			D_( std::cout << "trying to make room for " << id << std::endl);
			makeRoomFor(id, weight);
		}
		if (full())
			return false;
		addToCache(id, weight, data);
		return true;
	}

	bool get(const id_type &id, data_type &data) const {
		const CacheConstItr itr = m_Cache.find(id);
		if (itr == m_Cache.end())
			return false;
		data = itr->second.data;
		return true;
	}

	inline void setMaxWeight(const metric_type size) {
		m_MaxWeight = size;
	}
private:
	inline void dump(const char* dumpMessage) const {
#ifdef DEBUG_CACHE
		using namespace std;
		cout << dumpMessage << endl;
		cout << "pendings {" << endl;
		display(m_PendingIds);
		cout << "}" << endl;
		cout << "discardables {" << endl;
		display(m_DiscardableIds);
		cout << "}" << endl;
		cout << "----------------------------------------" << endl;
#endif
	}

	inline static bool in(const IdContainer &container, const id_type &value) {
		return std::find(container.begin(), container.end(), value) != container.end();
	}

	inline static bool remove(IdContainer &container, const id_type &value) {
		IdItr itr = std::remove(container.begin(), container.end(), value);
		if (itr == container.end())
			return false;
		container.erase(itr, container.end());
		return true;
	}

	inline bool remove(const id_type &value) {
		return remove(m_PendingIds, value) || remove(m_DiscardableIds, value);
	}

	inline metric_type contiguousWeight() const {
		metric_type sum = 0;
		const CacheConstItr &end = m_Cache.end();
		for (const auto& id : m_PendingIds) {
			const CacheConstItr &itr = m_Cache.find(id);
			if (itr == end)
				return sum;
			sum += itr->second.weight;
		}
		return sum;
	}

	metric_type currentWeight() const {
		metric_type sum = 0;
		for (const auto &pair : m_Cache)
			sum += pair.second.weight;
		return sum;
	}

	inline bool canFit(const metric_type weight) const {
		if (weight > m_MaxWeight)
			return false;
		const metric_type maxWeight = m_MaxWeight - weight;
		return currentWeight() <= maxWeight;
	}

	void makeRoomFor(const id_type currentId, const metric_type weight) {
		D_( std::cout << "{ " << currentWeight() << std::endl);

		const IdItr firstMissing = std::find_if(m_PendingIds.begin(), m_PendingIds.end(), [&](const id_type& id){
			return m_Cache.find(id) == m_Cache.end();
		});

		IdContainer discardables(firstMissing, m_PendingIds.end());
		discardables.insert(discardables.end(), m_DiscardableIds.begin(), m_DiscardableIds.end());

		const metric_type maxWeight = m_MaxWeight - weight;
		std::reverse(discardables.begin(), discardables.end());
		for (const auto &id : discardables) {
			evict(id);
			if (currentWeight() <= maxWeight)
				break;
		} //
		D_( std::cout << "} " << currentWeight() << std::endl);
	}

	inline void evict(id_type id) {
		CacheItr itr = m_Cache.find(id);
		if (itr == m_Cache.end())
			return; // not found
		m_Cache.erase(itr);
		remove(id);
		D_( std::cout << "\t- " << id << std::endl);
	}

	inline void addToCache(const id_type &id, const metric_type weight, const data_type &data) {
		if (!(in(m_PendingIds, id) || in(m_DiscardableIds, id)))
			m_DiscardableIds.push_back(id);
		m_Cache.insert(std::make_pair(id, WeightedData(weight, data)));
		D_( std::cout << "+ " << id << std::endl);
	}

private:
	metric_type m_MaxWeight;
	IdContainer m_DiscardableIds;
	IdContainer m_PendingIds;
	CacheContainer m_Cache;
};

} // namespace cache
} // namespace concurrent

#endif /* PRIORITYCACHE_DETAILS_HPP_ */
