#include <concurrent/cache/priority_cache.hpp>

#include <gtest/gtest.h>

#include <iostream>

using namespace std;
using namespace concurrent::cache;

typedef priority_cache<size_t, size_t, int> CACHE;

TEST(Cache, cacheFullness )
{
	EXPECT_TRUE( CACHE(0).full() );
    EXPECT_FALSE( CACHE(1).full() );
}

TEST(Cache, cacheBasics )
{
    CACHE cache(10);
    EXPECT_FALSE( cache.pending(0) );
    EXPECT_FALSE( cache.contains(0) );
    EXPECT_EQ( NEEDED, cache.update(0)  ); // creating
    EXPECT_EQ( NOT_NEEDED, cache.update(0)); // creating
    EXPECT_TRUE( cache.pending(0) );// now pending
    EXPECT_FALSE( cache.contains(0) );// but still not present
    EXPECT_TRUE( cache.put(0,1,-1) );// putting
    EXPECT_TRUE( cache.contains(0) );// now present

    EXPECT_EQ( 1u, cache.weight() );// now present

    CACHE::data_type data;
    EXPECT_TRUE( cache.get(0, data) );// getting is ok
    EXPECT_EQ( -1, data );// data is correct
}

TEST(Cache, noWeight )
{
    CACHE cache(1);
    EXPECT_THROW( cache.put(0,0,-1), std::logic_error ); // no weight
}

TEST(Cache, putEvenIfNotRequestedCanFit )
{
    CACHE cache(1);
    // putting an unneeded element
    EXPECT_TRUE( cache.put(5,1,-1) );
    EXPECT_TRUE( cache.contains(5) );// now present

    // a new request would be priority and discard the previous one
    cache.update(0);// requesting 0
    EXPECT_TRUE( cache.put(0,1,2) );// should be accepted
    EXPECT_TRUE( cache.contains(0) );// should be present
    EXPECT_FALSE( cache.contains(5) );// should be discarded
}

TEST(Cache, putEvenIfNotRequestedButCantFit )
{
    CACHE cache(1);
    cache.update(0); // requesting 0
    EXPECT_TRUE( cache.put(0,2,2) );// should be accepted
    EXPECT_TRUE( cache.contains(0) );// should be present

    // putting an unneeded element that can't fit
    EXPECT_FALSE( cache.put(5,1,-1) );// not added
    EXPECT_FALSE( cache.contains(5) );// not present
}

TEST(Cache, alreadyInCache )
{
    CACHE cache(1);
    cache.update(0);
    cache.put(0,1,-1);
    EXPECT_THROW( cache.put(0,1,-1), std::logic_error ); // already in cache
}

TEST(Cache, cacheFull )
{
    CACHE cache(10);
    cache.update(0);
    cache.update(1);
    EXPECT_FALSE( cache.full() ); // not yet full
    EXPECT_TRUE( cache.put(0,11,-1) );
    EXPECT_TRUE( cache.full() );// full
    EXPECT_EQ( 11u, cache.weight() );// now present
    EXPECT_FALSE( cache.put(1,1,1) );// can't put, we're full here
    EXPECT_FALSE( cache.contains(1) );// data is not here
    CACHE::data_type data;
    EXPECT_FALSE( cache.get(1, data) );// can't get 1
}

TEST(Cache, fullButHigherPriorityDiscardsRequested )
{
    CACHE cache(1);

    // expected [0,1,2]
    cache.update(0);
    cache.update(1);
    cache.update(2);

    // [_,_,2]
    EXPECT_TRUE( cache.put(2,2,0) ); // pushing 2
    EXPECT_FALSE( cache.full() ); // not full, 0 is not there
    EXPECT_TRUE( cache.contains(2) );

    // [_,1,_]
    EXPECT_TRUE( cache.put(1,2,0) ); // pushing 1, removes 2
    EXPECT_FALSE( cache.full() ); // not full, 0 is not there
    EXPECT_TRUE( cache.contains(1) );
    EXPECT_FALSE( cache.contains(2) );

    // [0,_,_]
    EXPECT_TRUE( cache.put(0,2,0) ); // pushing 0, removes 1
    EXPECT_TRUE( cache.full() ); // 0 is here, and cache is full
    EXPECT_TRUE( cache.contains(0) );
    EXPECT_FALSE( cache.contains(1) );
}

TEST(Cache, discardPendings )
{
    CACHE cache(10);

    cache.update(0);
    cache.update(1);
    cache.update(3);
    // requested [0,1,3], discardable []
    //           [_,_,_]              []

    cache.put(1,2,42);
    // requested [0,1,3], discardable []
    //           [_,X,_]              []

    cache.discardPending();
    // requested [], discardable [0,1,3]
    //           []              [_,X,_]

    EXPECT_EQ( 2U, cache.weight() );
    // jobs are not pending anymore
    EXPECT_FALSE( cache.pending(0) );
    EXPECT_FALSE( cache.pending(1) );
    EXPECT_FALSE( cache.pending(3) );
    // but content is here
    EXPECT_TRUE( cache.contains(1) );

    CACHE::data_type data;
    EXPECT_TRUE( cache.get(1, data) );
    EXPECT_EQ( 42, data );

    EXPECT_EQ( NOT_NEEDED, cache.update(1) );
    // requested [1], discardable [0,3]
    //           [X]              [_,_]
}
