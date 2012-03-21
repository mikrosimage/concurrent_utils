/*
 * Acknowledge.hpp
 *
 *  Created on: Mar 21, 2012
 *      Author: Guillaume Chatelet
 */

#ifndef RESPONSE_HPP_
#define RESPONSE_HPP_

#include "ConcurrentSlot.hpp"

namespace concurrent {

struct Response : private ConcurrentSlot<bool> {
    void ack() {
        ConcurrentSlot<bool>::set(true);
    }

    void wait() {
        bool dummy;
        ConcurrentSlot<bool>::waitGet(dummy);
    }
};

}  // namespace concurrent

#endif /* RESPONSE_HPP_ */
