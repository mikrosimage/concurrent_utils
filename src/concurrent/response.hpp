/*
 * Acknowledge.hpp
 *
 *  Created on: Mar 21, 2012
 *      Author: Guillaume Chatelet
 */

#ifndef RESPONSE_HPP_
#define RESPONSE_HPP_

#include "slot.hpp"

namespace concurrent {

struct response : private slot<bool> {
    void ack() {
        slot<bool>::set(true);
    }

    void wait() {
        bool dummy;
        slot<bool>::waitGet(dummy);
    }
};

}  // namespace concurrent

#endif /* RESPONSE_HPP_ */
