#include <concurrent/queue_adaptor.hpp>
#include <concurrent/queue.hpp>

#include <gtest/gtest.h>

#include <list>
#include <vector>
#include <algorithm>

using namespace std;

typedef concurrent::queue<int> IntQueue;

TEST(ConcurrentQueue,pushPop ) {
	IntQueue q;
	q.push(5);
	int unused;
	EXPECT_TRUE( q.tryPop(unused));
	// can pop
	EXPECT_EQ( unused, 5);
	// value is 5
}

TEST(ConcurrentQueue, clean ) {
	IntQueue q;
	q.push(5);
	q.clear();
	int unused;
	EXPECT_FALSE( q.tryPop(unused));
	// queue is empty
}

TEST(ConcurrentQueue, drainToCompatible ) {
	typedef list<int> IntList;
	typedef vector<int> IntVector;
	const IntList initialValues = { 5, 2, 3, -1, 6, 9, 10, 55 };

	{
		/**
		 * Pushing elements one by one
		 */
		IntQueue queue;

		concurrent::queue_adapter<IntQueue> adapted(queue);
		copy(initialValues.begin(), initialValues.end(), back_inserter(adapted));

		IntVector result;
		queue.drainTo(result);

		EXPECT_TRUE(equal(initialValues.begin(), initialValues.end(), result.begin()));
		int unused;
		EXPECT_FALSE( queue.tryPop(unused));
		// queue is empty
	}
	{
		/**
		 * Pushing elements at once
		 */
		IntList mutableCopy(initialValues);
		IntQueue queue;
		queue.drainFrom(mutableCopy);
		EXPECT_TRUE( mutableCopy.empty());
		// source is empty

		IntVector result;
		queue.drainTo(result);

		EXPECT_TRUE(equal(initialValues.begin(), initialValues.end(), result.begin()));
		int unused;
		EXPECT_FALSE( queue.tryPop(unused));
		// queue is empty
	}
}

