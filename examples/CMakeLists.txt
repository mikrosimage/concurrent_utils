ADD_EXECUTABLE(BoundedQueueSingleWorker BoundedQueueSingleWorker.cpp)
ADD_EXECUTABLE(ConcurrentSlot ConcurrentSlot.cpp)
ADD_EXECUTABLE(LookAheadCache LookAheadCache.cpp)
ADD_EXECUTABLE(QueueManyWorkers QueueManyWorkers.cpp)
ADD_EXECUTABLE(QueueSingleWorker QueueSingleWorker.cpp)

#LINK libraries
TARGET_LINK_LIBRARIES(BoundedQueueSingleWorker ${Boost_LIBRARIES})
TARGET_LINK_LIBRARIES(ConcurrentSlot ${Boost_LIBRARIES})
TARGET_LINK_LIBRARIES(LookAheadCache ${Boost_LIBRARIES})
TARGET_LINK_LIBRARIES(QueueManyWorkers ${Boost_LIBRARIES})
TARGET_LINK_LIBRARIES(QueueSingleWorker ${Boost_LIBRARIES})