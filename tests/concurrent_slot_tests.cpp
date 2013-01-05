/*
 * concurrent_slot_tests.cpp
 *
 *  Created on: Mar 21, 2012
 *      Author: Guillaume Chatelet
 */

#include <concurrent/slot.hpp>

#include <gtest/gtest.h>

using namespace concurrent;

TEST(ConcurrentSlot,uninitialized ) {
	bool dummy;
	slot<bool> uninitialized;
	EXPECT_FALSE(uninitialized.tryGet(dummy));
}

TEST(ConcurrentSlot, initialized ) {
	bool dummy = false;
	slot<bool> initialized(true);
	// taking value
	EXPECT_TRUE(initialized.tryGet(dummy));
	EXPECT_TRUE(dummy);
	// no more value available
	EXPECT_FALSE(initialized.tryGet(dummy));
}

TEST(ConcurrentSlot, termination ) {
	bool dummy = false;
	slot<bool> slot(true);
	slot.terminate();
	// termination mode, getters should throw
	EXPECT_THROW(slot.tryGet(dummy), concurrent::terminated);
	EXPECT_THROW(slot.waitGet(dummy), concurrent::terminated);
	// back to normal operations
	slot.terminate(false);
	EXPECT_TRUE(slot.tryGet(dummy));
	EXPECT_TRUE(dummy);
	slot.set(false);
	slot.waitGet(dummy);
	EXPECT_FALSE(dummy);
}

