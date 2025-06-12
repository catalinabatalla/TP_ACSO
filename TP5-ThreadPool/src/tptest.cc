#include <iostream>
#include "thread-pool.h"
#include <mutex>
#include <atomic>
using namespace std;

void sleep_for(int slp){
    this_thread::sleep_for(chrono::milliseconds(slp));
}

static mutex oslock;
static atomic<size_t> startedCount(0);
static atomic<size_t> finishedCount(0);

static const size_t kNumThreads = 12;
static const size_t kNumFunctions = 1000;

static void simpleTest() {
  ThreadPool pool(kNumThreads);
  for (size_t id = 0; id < kNumFunctions; id++) {
    pool.schedule([id] {
      {
        lock_guard<mutex> lock(oslock);
        cout << "Thread (ID: " << id << ") has started." << endl;
      }
      startedCount++;

      size_t sleepTime = (id % 3) * 10;
      sleep_for(sleepTime);

      {
        lock_guard<mutex> lock(oslock);
        cout << "Thread (ID: " << id << ") has finished." << endl;
      }
      finishedCount++;
    });
  }

  pool.wait();

  // Imprimir resumen final
  cout << "\nSummary:\n";
  cout << "Total threads scheduled: " << kNumFunctions << endl;
  cout << "Threads started: " << startedCount.load() << endl;
  cout << "Threads finished: " << finishedCount.load() << endl;

  if (startedCount == kNumFunctions && finishedCount == kNumFunctions) {
    cout << "All threads started and finished correctly!" << endl;
  } else {
    cout << "Mismatch in started/finished thread counts!" << endl;
  }
}

int main(int argc, char *argv[]) {
  simpleTest();
  return 0;
}
