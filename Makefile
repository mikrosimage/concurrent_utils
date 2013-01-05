CC=g++
CFLAGS=-I. -Wall -fmessage-length=0 -std=c++11
CFLAGS+=-D_GLIBCXX_USE_NANOSLEEP # g++ std::this_thread::sleep_for in benchmark
CFLAGS+=-O0 -g
LDFLAGS=-lpthread  

.PHONY: clean examples

all:examples test

EXAMPLES=BoundedQueueSingleWorker ConcurrentSlot LookAheadCache QueueManyWorkers QueueSingleWorker

examples: $(EXAMPLES)

BoundedQueueSingleWorker: examples/BoundedQueueSingleWorker.cpp
	$(CC) $(CFLAGS) $(LDFLAGS) -o BoundedQueueSingleWorker $^
	
ConcurrentSlot: examples/ConcurrentSlot.cpp
	$(CC) $(CFLAGS) $(LDFLAGS) -o ConcurrentSlot $^
	
LookAheadCache: examples/LookAheadCache.cpp
	$(CC) $(CFLAGS) $(LDFLAGS) -o LookAheadCache $^
	
QueueManyWorkers: examples/QueueManyWorkers.cpp
	$(CC) $(CFLAGS) $(LDFLAGS) -o QueueManyWorkers $^
	
QueueSingleWorker: examples/QueueSingleWorker.cpp
	$(CC) $(CFLAGS) $(LDFLAGS) -o QueueSingleWorker $^

test:tests/*.cpp tests/benchmark/*.cpp
	$(CC) $(CFLAGS) $(LDFLAGS) -lgtest -lgtest_main -o test $^

clean:
	rm -f $(EXAMPLES) test
