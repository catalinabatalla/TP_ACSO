/**
 * File: thread-pool.h
 * -------------------
 * This class defines the ThreadPool class, which accepts a collection
 * of thunks (which are zero-argument functions that don't return a value)
 * and schedules them in a FIFO manner to be executed by a constant number
 * of child threads that exist solely to invoke previously scheduled thunks.
 */

#ifndef _thread_pool_
#define _thread_pool_

#include <cstddef>       // for size_t
#include <functional>    // for function
#include <thread>        // for thread
#include <vector>        // for vector
#include <queue>         // for queue
#include <mutex>         // for mutex
#include <condition_variable> // for condition_variable
#include "Semaphore.h"   // for Semaphore
#include <atomic>
#include <stdexcept>


using namespace std;


/**
 * @brief Represents a worker in the thread pool.
 */
typedef struct worker {
    thread ts;                        
    function<void(void)> thunk;      
    bool available = true;            
    mutex mtx;                        
    Semaphore ready{0};                
} worker_t;

class ThreadPool {
  public:
    ThreadPool(size_t numThreads);
    void schedule(const function<void(void)>& thunk);
    void wait();
    ~ThreadPool();

  private:
    void dispatcher();
    void worker(size_t id);

    vector<worker_t> wts;                  
    queue<function<void(void)>> taskQueue;       
    mutex queueLock;                        
    condition_variable dispatcherCV;           
    thread dispatcherThread;                 

    int activeTasks = 0;                      
    mutex waitLock;                         
    condition_variable waitCV;                    
    

    std::atomic<bool> done{false};     

    // Prevent copying
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
};

#endif