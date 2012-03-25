#include <concurrent/queue.hpp>
#include <concurrent/cache/lookahead_cache.hpp>

#include <boost/thread.hpp>

#include <deque>
#include <fstream>

//#define BOOST_TEST_MODULE BenchmarkTestModule
//#include <boost/test/unit_test.hpp>

using namespace std;
using namespace concurrent::cache;
using namespace boost::posix_time;

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
    Job() {
    }

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

//BOOST_AUTO_TEST_SUITE( BenchmarkTestSuite )

concurrent::queue<id_type> decodeQueue;

inline static void sleepFor(const size_t ms) {
    boost::this_thread::sleep(boost::posix_time::millisec(ms));
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
                jobProducer.putWorkItem(pUnit, 1, 0);
            } else {
                jobProducer.popWorkItem(pUnit);
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
        jobProducer.putWorkItem(pUnit, 1, 0);
    }
}

deque<JobData> loadData(const char* filename) {
    deque<JobData> data;
    ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("unable to load file");
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

static inline time_duration launchBench(const char *filename, const size_t threads) {
    CACHE cache(-1); // unlimited cache
    const deque<JobData> data = loadData(filename);

    // launching the worker
    boost::thread_group group;
    for (size_t i = 0; i < threads; ++i)
        group.create_thread(boost::bind(&worker, boost::ref(cache)));

    // getting time before
    ptime start(microsec_clock::local_time());

    //pushing the job
    cache.pushNewJob(Job(data));

    // waiting for worker to stop
    group.join_all();
    ptime end(microsec_clock::local_time());

    return end - start;
}

int main(int argc, char **argv) {
    const size_t max_thread = 16;
    vector<string> filenames;

    cout << "loading data files : ";
    for (int i = 1; i < argc; ++i) {
        filenames.push_back(argv[i]);
        cout << filenames.back() << " ";
    }
    cout << endl;

    typedef map<size_t, time_duration> Times;
    typedef map<string, Times> TimeMap;
    TimeMap data;
    for (vector<string>::const_iterator itr = filenames.begin(); itr != filenames.end(); ++itr) {
        const string &filename = *itr;
        Times times;
        cout << "performing test for " << filename << endl;
        for (size_t i = 1; i <= max_thread; ++i) {
            cout << "*thread " << i << " : ";
            cout.flush();
            times.insert(make_pair(i, launchBench(filename.c_str(), i)));
            cout << times.rbegin()->second.total_milliseconds() << "ms" << endl;
        }
        data.insert(make_pair(filename, times));
    }

    // analyzing the results
    for (TimeMap::const_iterator itr = data.begin(); itr != data.end(); ++itr) {
        const TimeMap::value_type &filenamePair = *itr;
        cout << filenamePair.first << endl;
        time_duration reference;
        const Times &times = filenamePair.second;
        for (Times::const_iterator timeItr = times.begin(); timeItr != times.end(); ++timeItr) {
            const Times::value_type &timePair = *timeItr;
            const time_duration &time = timePair.second;
            cout << '#' << timePair.first << '\t' << time.total_milliseconds() << " ms";
            if (timePair.first == 1) {
                reference = time;
            } else {
                cout << "\t speedup x" << (double(reference.total_milliseconds()) / time.total_milliseconds());
            }
            cout << endl;
        }
    }
}

//BOOST_AUTO_TEST_SUITE_END()
