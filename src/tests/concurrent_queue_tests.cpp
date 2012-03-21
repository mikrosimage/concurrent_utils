#include <concurrent/QueueAdaptor.hpp>
#include <concurrent/Queue.hpp>

#include <list>
#include <vector>

#include <boost/assign/std/vector.hpp>
#include <boost/assign/list_of.hpp>

#define BOOST_TEST_MODULE ConcurrentQueueTestModule
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace concurrent;

typedef Queue<int> IntConcurrentQueue;

BOOST_AUTO_TEST_SUITE( ConcurrentQueueTestSuite )

BOOST_AUTO_TEST_CASE( pushPop )
{
    IntConcurrentQueue queue;
    queue.push(5);
    int unused;
    BOOST_CHECK( queue.tryPop(unused) ); // can pop
    BOOST_CHECK_EQUAL( unused, 5 ); // value is 5
}

BOOST_AUTO_TEST_CASE( clean )
{
    IntConcurrentQueue queue;
    queue.push(5);
    queue.clear();
    int unused;
    BOOST_CHECK( !queue.tryPop(unused) ); // queue is empty
}

BOOST_AUTO_TEST_CASE( drainToCompatible )
{
    typedef list<int> IntList;
    typedef vector<int> IntVector;
    const IntList initialValues= boost::assign::list_of(5)(2)(3)(-1)(6)(9)(10)(55);

    {
        /**
         * Pushing elements one by one
         */
        IntConcurrentQueue queue;

        QueueAdapter<IntConcurrentQueue> adapted(queue);
        copy(initialValues.begin(), initialValues.end(), back_inserter(adapted));

        IntVector result;
        queue.drainTo(result);

        BOOST_CHECK_EQUAL_COLLECTIONS(initialValues.begin(), initialValues.end(), result.begin(), result.end());
        int unused;
        BOOST_CHECK( !queue.tryPop(unused) ); // queue is empty
    }
    {
        /**
         * Pushing elements at once
         */
        IntList mutableCopy(initialValues);
        IntConcurrentQueue queue;
        queue.drainFrom(mutableCopy);
        BOOST_CHECK( mutableCopy.empty() ); // source is empty

        IntVector result;
        queue.drainTo(result);

        BOOST_CHECK_EQUAL_COLLECTIONS(initialValues.begin(), initialValues.end(), result.begin(), result.end());
        int unused;
        BOOST_CHECK( !queue.tryPop(unused) ); // queue is empty
    }
}

BOOST_AUTO_TEST_SUITE_END()
