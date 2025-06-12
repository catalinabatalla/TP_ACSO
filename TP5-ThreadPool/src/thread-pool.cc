/**
 * File: thread-pool.cc
 * --------------------
 * Presents the implementation of the ThreadPool class.
 */

#include "thread-pool.h"
using namespace std;

ThreadPool::ThreadPool(size_t numThreads) : wts(numThreads), done(false) {
        for (size_t i = 0; i < numThreads; ++i) {
        wts[i].available = true;
        wts[i].ts = thread(&ThreadPool::worker, this, i);
    }
    dispatcherThread = thread(&ThreadPool::dispatcher, this);
}


void ThreadPool::schedule(const function<void(void)>& thunk) {
    {
        lock_guard<mutex> lg(queueLock);
        taskQueue.push(thunk);
    }
    dispatcherCV.notify_one();  // avisamos al dispatcher que hay trabajo
}

void ThreadPool::dispatcher() {
    while (true) {
        function<void(void)> task;

        {
            unique_lock<mutex> ul(queueLock);
            dispatcherCV.wait(ul, [this]() {
                return done || !taskQueue.empty();
            });

            if (done && taskQueue.empty()) break;

            if (taskQueue.empty()) continue;

            task = taskQueue.front();
            taskQueue.pop();
        }

        // Esperar un worker disponible
        while (true) {
            if (done) break;
            for (size_t i = 0; i < wts.size(); ++i) {
                worker_t& worker = wts[i];

                unique_lock<mutex> lk(worker.mtx);
                if (worker.available) {
                    worker.available = false;
                    worker.thunk = task;

                    {
                        lock_guard<mutex> lock(waitLock);
                        activeTasks++;
                    }

                    worker.ready.signal();
                    lk.unlock();
                    goto NEXT_TASK;
                }
            }
            this_thread::yield();  // ceder CPU hasta que un worker se libere
        }

    NEXT_TASK:;
    }
}

void ThreadPool::worker(size_t id) {
    while (true) {
        wts[id].ready.wait();  // espera que el dispatcher lo despierte

        if (done){
            break; // para finalizar si ya cerramos
        }

        function<void(void)> task;
        {
            lock_guard<mutex> lg(wts[id].mtx);
            task = wts[id].thunk;
        }

        if (done) break;

        task();  // ejecutar función

        {
            lock_guard<mutex> lk(waitLock);
            activeTasks--;
            if (activeTasks == 0 && taskQueue.empty()) {
                waitCV.notify_all();  // notificar si no queda nada
            }
        }

        {
            lock_guard<mutex> lk(wts[id].mtx);
            wts[id].thunk = nullptr;  // limpiar tarea
            wts[id].available = true;
        }
    }
}

void ThreadPool::wait() {
    unique_lock<mutex> lk(waitLock);
    waitCV.wait(lk, [this]() {
        return taskQueue.empty() && activeTasks == 0;
    });
}

ThreadPool::~ThreadPool() {
    wait();  // esperar que termine todo

    {
        lock_guard<mutex> lg(queueLock);
        done = true;
    }

    dispatcherCV.notify_all();  // despertar dispatcher
    dispatcherThread.join();

    for (auto& wt : wts) {
        wt.ready.signal();      // despertar cada worker
        wt.ts.join();           // esperar su finalización
    }
}
