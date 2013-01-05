#include <concurrent/queue.hpp>
#include <concurrent/cache/lookahead_cache.hpp>

#include <gtest/gtest.h>

#include <chrono>
#include <thread>
#include <deque>
#include <fstream>

using namespace std;
using namespace chrono;
using namespace concurrent::cache;

struct JobData {
	size_t loadTime;
	size_t decodeTime;

	JobData() :
			loadTime(-1), decodeTime(-1) {
	}

	inline bool operator<(const JobData& other) const {
		if (loadTime == other.loadTime)
			return decodeTime < other.decodeTime;
		return loadTime < other.loadTime;
	}
};

typedef JobData* id_type;

struct Job {
	Job() = default;

	Job(const deque<JobData> &data) :
	m_Data(data), m_Index(0) {
	}

	inline void clear() {
		m_Index = m_Data.size();
	}

	inline bool empty() const {
		return m_Index >= m_Data.size();
	}

	inline id_type next() {
		return &m_Data[m_Index++];
	}

private:
	deque<JobData> m_Data;
	size_t m_Index;
};

typedef size_t metric_type;
typedef size_t data_type;
typedef lookahead_cache<id_type, metric_type, data_type, Job> CACHE;

concurrent::queue<id_type> decodeQueue;

inline static void sleepFor(const size_t ms) {
	this_thread::sleep_for(milliseconds(ms));
}

inline static void decode(const JobData &unit) {
	sleepFor(unit.decodeTime);
}

inline static void load(const JobData &unit) {
	sleepFor(unit.loadTime);
}

inline static bool lastUnit(const JobData &unit) {
	return unit.loadTime == size_t(-1) && unit.decodeTime == size_t(-1);
}

void worker(CACHE &jobProducer) {
	JobData *pUnit = NULL;
	try {
		while (true) {
			if (decodeQueue.tryPop(pUnit)) {
				decode(*pUnit);
				jobProducer.push(pUnit, 1, 0);
			} else {
				jobProducer.pop(pUnit);
				if (lastUnit(*pUnit)) {
					jobProducer.terminate();
					break;
				} else {
					load(*pUnit);
					decodeQueue.push(pUnit);
				}
			}
		}
	} catch (concurrent::terminated &e) {
	}
	while (decodeQueue.tryPop(pUnit)) {
		decode(*pUnit);
		jobProducer.push(pUnit, 1, 0);
	}
}

deque<JobData> loadData(const char* filename) {
	deque<JobData> data;
	ifstream file(filename);
	if (!file.is_open())
		throw runtime_error("unable to load file");
	while (file.good()) {
		JobData entry;
		file >> entry.loadTime;
		file >> entry.decodeTime;
		data.push_back(entry);
	}
	// adding sentinel
	data.push_back(JobData());
	return data;
}

static inline milliseconds launchBench(const char *filename, const size_t threads) {
	CACHE cache(-1); // unlimited cache
	const deque<JobData> data = loadData(filename);

	// launching the worker
	vector<thread> group;
	for (size_t i = 0; i < threads; ++i)
		group.emplace_back(bind(&worker, ref(cache)));

	// getting time before
	const auto start = high_resolution_clock::now();

	//pushing the job
	cache.process(Job(data));

	// waiting for worker to stop
	for (thread &thread : group)
		thread.join();
	const auto end = high_resolution_clock::now();

	return duration_cast<milliseconds>(end - start);
}

TEST(cache, DISABLED_benchmark) {
	const size_t max_thread = 16;
	const vector<string> filenames = { "tests/benchmark/data/gch.txt", "tests/benchmark/data/nro.txt" };

	typedef map<size_t, milliseconds> Times;
	typedef map<string, Times> TimeMap;
	TimeMap data;
	for (const string& filename : filenames) {
		Times times;
		cout << "performing test for " << filename << endl;
		for (size_t i = 1; i <= max_thread; ++i) {
			cout << "*thread " << i << " : ";
			cout.flush();
			times.insert(make_pair(i, launchBench(filename.c_str(), i)));
			cout << times.rbegin()->second.count() << "ms" << endl;
		}
		data.insert(make_pair(filename, times));
	}

	// analyzing the results
	for (const auto&filenamePair : data) {
		cout << filenamePair.first << endl;
		milliseconds reference;
		const Times &times = filenamePair.second;
		for (const Times::value_type &timePair : times) {
			const milliseconds &time = timePair.second;
			cout << '#' << timePair.first << '\t' << time.count() << " ms";
			if (timePair.first == 1) {
				reference = time;
			} else {
				cout << "\t speedup x" << (double(reference.count()) / time.count());
			}
			cout << endl;
		}
	}
}
