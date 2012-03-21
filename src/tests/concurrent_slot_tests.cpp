/*
 * concurrent_slot_tests.cpp
 *
 *  Created on: Mar 21, 2012
 *      Author: Guillaume Chatelet
 */

#include <concurrent/Slot.hpp>

#define BOOST_TEST_MODULE ConcurrentSlotTestModule
#include <boost/test/unit_test.hpp>

using namespace concurrent;


BOOST_AUTO_TEST_SUITE( ConcurrentSlotTestSuite )

BOOST_AUTO_TEST_CASE( uninitialized )
{
    bool dummy;
    Slot<bool> uninitialized;
    BOOST_CHECK( ! uninitialized.tryGet(dummy) );
}

BOOST_AUTO_TEST_CASE( initialized )
{
    bool dummy = false;
    Slot<bool> initialized(true);
    // taking value
    BOOST_CHECK( initialized.tryGet(dummy) );
    BOOST_CHECK_EQUAL( true, dummy );
    // no more value available
    BOOST_CHECK( ! initialized.tryGet(dummy) );
}

BOOST_AUTO_TEST_CASE( termination )
{
    bool dummy = false;
    Slot<bool> slot(true);
    slot.terminate();
    // termination mode, accessors should throw
    BOOST_CHECK_THROW( slot.set(false), concurrent::terminated );
    BOOST_CHECK_THROW( slot.tryGet(dummy), concurrent::terminated );
    BOOST_CHECK_THROW( slot.waitGet(dummy), concurrent::terminated );
    // back to normal operations
    slot.terminate(false);
    BOOST_CHECK( slot.tryGet(dummy) );
    BOOST_CHECK_EQUAL( true, dummy );
    slot.set(false);
    slot.waitGet(dummy);
    BOOST_CHECK_EQUAL( false, dummy );
}

BOOST_AUTO_TEST_SUITE_END()
