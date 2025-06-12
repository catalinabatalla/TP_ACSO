#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include <functional>
#include <cstring>
#include <mutex>
#include <atomic>
#include <vector>
#include <sys/types.h> // usado para contar hilos (no se usa en el ejemplo)
#include <unistd.h>    // idem
#include <dirent.h>    // idem

#include "thread-pool.h"

using namespace std;

void sleep_for(int slp){
    this_thread::sleep_for(chrono::milliseconds(slp));
}

static mutex oslock;

static const size_t kNumThreads = 4;
static const size_t kNumFunctions = 10;

static void simpleTest() {
  cout << "--- simpleTest ---" << endl;
  ThreadPool pool(kNumThreads);
  for (size_t id = 0; id < kNumFunctions; id++) {
    pool.schedule([id] {
      oslock.lock();
      cout << "Thread (ID: " << id << ") has started." << endl;
      oslock.unlock();
      size_t sleepTime = (id % 3) * 10;
      sleep_for(sleepTime);
      oslock.lock();
      cout <<  "Thread (ID: " << id << ") has finished." << endl ;
      oslock.unlock();
    });
  }
  cout << "Waiting for all tasks in simpleTest..." << endl;
  pool.wait();
  cout << "simpleTest completed." << endl << endl;
}

static void singleThreadNoWaitTest() {
  cout << "--- singleThreadNoWaitTest ---" << endl;
  ThreadPool pool(4);
  pool.schedule([&] {
    oslock.lock();
    cout << "This is a test." << endl;
    oslock.unlock();
  });
  cout << "No wait called, sleeping 1s..." << endl;
  sleep_for(1000);
  cout << "singleThreadNoWaitTest finished (without wait)." << endl << endl;
}

static void singleThreadSingleWaitTest() {
  cout << "--- singleThreadSingleWaitTest ---" << endl;
  ThreadPool pool(4);
  pool.schedule([] {
    oslock.lock();
    cout << "This is a test." << endl;
    oslock.unlock();
    sleep_for(1000);
  });
  cout << "Calling wait()..." << endl;
  pool.wait();
  cout << "Wait returned." << endl << endl;
}

static void noThreadsDoubleWaitTest() {
  cout << "--- noThreadsDoubleWaitTest ---" << endl;
  ThreadPool pool(4);
  cout << "Calling wait() first time..." << endl;
  pool.wait();
  cout << "Calling wait() second time..." << endl;
  pool.wait();
  cout << "Both waits returned." << endl << endl;
}

static void reuseThreadPoolTest() {
  cout << "--- reuseThreadPoolTest ---" << endl;
  ThreadPool pool(4);
  cout << "Scheduling 16 tasks..." << endl;
  for (size_t i = 0; i < 16; i++) {
    pool.schedule([] {
      oslock.lock();
      cout << "This is a test." << endl;
      oslock.unlock();
      sleep_for(50);
    });
  }
  cout << "Waiting for first batch..." << endl;
  pool.wait();
  cout << "Scheduling 1 more task..." << endl;
  pool.schedule([] {
    oslock.lock();
    cout << "This is a code." << endl;
    oslock.unlock();
    sleep_for(1000);
  });
  cout << "Waiting for second batch..." << endl;
  pool.wait();
  cout << "reuseThreadPoolTest finished." << endl << endl;
}

// Nuevo test: tareas rápidas
static void fastTasksTest() {
  cout << "--- fastTasksTest ---" << endl;
  ThreadPool pool(4);
  atomic<int> started(0), finished(0);
  for (int i = 0; i < 20; i++) {
    pool.schedule([&started, &finished, i] {
      started++;
      oslock.lock();
      cout << "Fast task " << i << " started." << endl;
      oslock.unlock();
      // No sleep, tarea instantánea
      finished++;
      oslock.lock();
      cout << "Fast task " << i << " finished." << endl;
      oslock.unlock();
    });
  }
  pool.wait();
  cout << "Total started: " << started << ", finished: " << finished << endl << endl;
}

// Nuevo test: más tareas que threads (stress)
static void stressTest() {
  cout << "--- stressTest ---" << endl;
  constexpr size_t NTHREADS = 8;
  constexpr size_t NTASKS = 100;
  ThreadPool pool(NTHREADS);
  atomic<int> count(0);

  for (size_t i = 0; i < NTASKS; i++) {
    pool.schedule([&count, i] {
      oslock.lock();
      cout << "Stress task " << i << " running." << endl;
      oslock.unlock();
      sleep_for(10);
      count++;
    });
  }

  cout << "Waiting for stress tasks..." << endl;
  pool.wait();
  cout << "Stress test complete. Tasks done: " << count << "/" << NTASKS << endl << endl;
}

// Nuevo test: tarea con variable compartida y mutex
static void sharedVariableTest() {
  cout << "--- sharedVariableTest ---" << endl;
  ThreadPool pool(4);
  int sharedVal = 0;
  mutex mtx;

  for (int i = 0; i < 20; i++) {
    pool.schedule([&sharedVal, &mtx, i] {
      this_thread::sleep_for(chrono::milliseconds(i * 5));
      lock_guard<mutex> lk(mtx);
      sharedVal += 1;
      oslock.lock();
      cout << "Incremented sharedVal to " << sharedVal << " by task " << i << endl;
      oslock.unlock();
    });
  }
  pool.wait();
  cout << "Final sharedVal: " << sharedVal << endl << endl;
}

struct testEntry {
  string flag;
  function<void(void)> testfn;
};

static void buildMap(map<string, function<void(void)>>& testFunctionMap) {
  testEntry entries[] = {
    {"--single-thread-no-wait", singleThreadNoWaitTest},
    {"--single-thread-single-wait", singleThreadSingleWaitTest},
    {"--no-threads-double-wait", noThreadsDoubleWaitTest},
    {"--reuse-thread-pool", reuseThreadPoolTest},
    {"--s", simpleTest},
    {"--fast", fastTasksTest},
    {"--stress", stressTest},
    {"--shared-var", sharedVariableTest},
  };

  for (const testEntry& entry: entries) {
    testFunctionMap[entry.flag] = entry.testfn;
  }
}

static void executeAll(const map<string, function<void(void)>>& testFunctionMap) {
  for (const auto& entry: testFunctionMap) {
    cout << entry.first << ":" << endl;
    entry.second();
  }
}

int main(int argc, char **argv) {
  if (argc != 2) {
    cout << "Ouch! I need exactly two arguments." << endl;
    cout << "Available flags:" << endl;
    cout << "  --all\n  --single-thread-no-wait\n  --single-thread-single-wait\n  --no-threads-double-wait\n  --reuse-thread-pool\n  --s\n  --fast\n  --stress\n  --shared-var" << endl;
    return 0;
  }

  map<string, function<void(void)>> testFunctionMap;
  buildMap(testFunctionMap);
  string flag = argv[1];
  if (flag == "--all") {
    executeAll(testFunctionMap);
    return 0;
  }
  auto found = testFunctionMap.find(argv[1]);
  if (found == testFunctionMap.end()) {
    cout << "Oops... we don't recognize the flag \"" << argv[1] << "\"." << endl;
    return 0;
  }

  found->second();
  return 0;
}
