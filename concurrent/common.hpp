/*
 * Common.hpp
 *
 *  Created on: Mar 21, 2012
 *      Author: Guillaume Chatelet
 */

#ifndef COMMON_HPP_
#define COMMON_HPP_

#include <stdexcept>

namespace concurrent {

/**
 * Exception thrown when a container will not serve more items
 */
struct terminated : public std::exception {};

struct noncopyable {
	noncopyable() = default;
	noncopyable(const noncopyable&) = delete;
	noncopyable & operator=(const noncopyable&) = delete;
};

}

#endif /* COMMON_HPP_ */
