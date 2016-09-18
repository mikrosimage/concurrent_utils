
ADD_EXECUTABLE(concurrent_slot_tests concurrent_slot_tests.cpp)
ADD_EXECUTABLE(concurrent_queue_tests concurrent_queue_tests.cpp)
ADD_EXECUTABLE(cache_tests cache_tests.cpp)

TARGET_LINK_LIBRARIES(concurrent_slot_tests ${Boost_LIBRARIES})
TARGET_LINK_LIBRARIES(concurrent_queue_tests ${Boost_LIBRARIES})
TARGET_LINK_LIBRARIES(cache_tests ${Boost_LIBRARIES})

ADD_TEST(concurrent_slot_tests concurrent_slot_tests)
ADD_TEST(concurrent_queue_tests concurrent_queue_tests)
ADD_TEST(cache_tests cache_tests)

ADD_SUBDIRECTORY(benchmark)