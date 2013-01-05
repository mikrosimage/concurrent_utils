/*
 * common_queue.hpp
 *
 *  Created on: Mar 22, 2012
 *      Author: Guillaume Chatelet
 */

#ifndef COMMON_QUEUE_HPP_
#define COMMON_QUEUE_HPP_

#include "call_type_traits.hpp"

#include <concurrent/common.hpp>
#include <mutex>
#include <iterator>

namespace concurrent {
namespace details {

/**
 * base implementation of the queues functionalities via Static Polymorphism
 * and use of the CRTP ( Curiously Recurring Template Pattern )
 */
template<typename Derived, typename Container>
struct queue_base: private noncopyable {
	typedef Container container_type;
	typedef typename container_type::size_type size_type;
	typedef typename container_type::value_type value_type;
	typedef typename call_traits<value_type>::reference reference;
	typedef typename call_traits<value_type>::param_type param_type;

	static_assert(std::is_trivial<size_type>::value,"size_type must be trivial");
	static_assert(std::is_trivial<value_type>::value,"value_type must be trivial");

	void push(param_type value) {
		std::unique_lock<std::mutex> lock(m_mutex);
		exact()->wait_not_full(lock);
		exact()->_push(value);
		exact()->notify_not_empty();
	}

	bool tryPush(param_type value) {
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!exact()->is_not_full())
			return false; // full
		exact()->_push(value);
		exact()->notify_not_empty();
		return true;
	}

	void pop(reference value) {
		std::unique_lock<std::mutex> lock(m_mutex);
		exact()->wait_not_empty(lock);
		value = exact()->_pop();
		exact()->notify_not_full();
	}

	bool tryPop(reference value) {
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!exact()->is_not_empty())
			return false; // empty
		value = exact()->_pop();
		exact()->notify_not_full();
		return true;
	}

	void clear() {
		std::lock_guard<std::mutex> lock(m_mutex);
		if (exact()->is_not_empty()) {
			exact()->_clear();
			exact()->notify_not_full();
		}
	}

	template<typename CompatibleContainer>
	void drainFrom(CompatibleContainer &collection) {
		if (collection.empty())
			return;
		std::lock_guard<std::mutex> lock(m_mutex);
		drain<CompatibleContainer, container_type>(collection, exact()->m_container);
		exact()->notify_not_empty();
	}

	template<typename CompatibleContainer>
	bool drainTo(CompatibleContainer& collection) {
		std::lock_guard<std::mutex> lock(m_mutex);
		if (exact()->is_not_empty()) {
			drain<container_type, CompatibleContainer>(exact()->m_container, collection);
			exact()->notify_not_full();
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
		std::copy(from.begin(), from.end(), std::back_inserter(to));
		from.clear();
	}

	std::mutex m_mutex;
};

} // namespace details
} // namespace concurrent

#endif /* COMMON_QUEUE_HPP_ */
