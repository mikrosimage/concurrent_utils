/*
 * Acknowledge.hpp
 *
 *  Created on: Mar 21, 2012
 *      Author: Guillaume Chatelet
 */

#ifndef RESPONSE_HPP_
#define RESPONSE_HPP_

#include "Slot.hpp"

namespace concurrent {

struct Response : private Slot<bool> {
    void ack() {
        Slot<bool>::set(true);
    }

    void wait() {
        bool dummy;
        Slot<bool>::waitGet(dummy);
    }
};

}  // namespace concurrent

#endif /* RESPONSE_HPP_ */
