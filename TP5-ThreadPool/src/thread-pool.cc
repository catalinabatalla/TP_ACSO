/**
 * File: thread-pool.cc
 * --------------------
 * Presents the implementation of the ThreadPool class.
 */

#include "thread-pool.h"
using namespace std;
// thread-pool.cc

ThreadPool::ThreadPool(size_t numThreads) : wts(numThreads), done(false) {
    for (size_t i = 0; i < numThreads; ++i) {
        wts[i].available = true;
        wts[i].ts = thread(&ThreadPool::worker, this, i);
    }
    dispatcherThread = thread(&ThreadPool::dispatcher, this);
}

void ThreadPool::schedule(const function<void(void)>& thunk) {
    if (done) throw runtime_error("Cannot schedule on a destroyed pool");
    if (!thunk) throw invalid_argument("Scheduled function is null");

    {
        lock_guard<mutex> lg(queueLock);
        taskQueue.push(thunk);
        dispatcherCV.notify_one();  
    }
}


void ThreadPool::dispatcher() {
    while (true) {
        function<void(void)> task;
        worker_t* availableWorker = nullptr;

        {
            unique_lock<mutex> ul(queueLock);

            // Espero que haya tareas o se termine
            dispatcherCV.wait(ul, [this]() {
                return done || !taskQueue.empty();
            });

            if (done && taskQueue.empty()) break;

            if (taskQueue.empty()) continue;

            // Espero worker disponible antes de sacar tarea
            while (!availableWorker && !done) {
                for (size_t i = 0; i < wts.size(); ++i) {
                    worker_t& worker = wts[i];
                    unique_lock<mutex> lk(worker.mtx);
                    if (worker.available) {
                        worker.available = false;
                        availableWorker = &worker;
                        break;  // worker encontrado
                    }
                }
                if (!availableWorker) {
                    ul.unlock();
                    this_thread::yield();
                    ul.lock();
                }
            }

            if (done) break;

            // Ahora que tengo worker disponible, saco la tarea, esto significa que
            // puedo asignar la tarea al worker y notificarlo
            task = taskQueue.front();
            taskQueue.pop();

            // Incremento contador de tareas activas
            activeTasks++;
        } // fin lock queueLock

        {
            // Asigno la tarea al worker con su mutex propio y desbloqueo
            unique_lock<mutex> lk(availableWorker->mtx);
            availableWorker->thunk = task;
        }

        // Señalo al worker que tiene tarea lista
        availableWorker->ready.signal();
    }
}

void ThreadPool::worker(size_t id) {
    while (true) {
        wts[id].ready.wait();

        if (done) break;

        function<void(void)> task;
        {
            lock_guard<mutex> lg(wts[id].mtx);
            task = wts[id].thunk;
        }

        if (task) task();

        {
            lock_guard<mutex> lk(queueLock);
            activeTasks--;
            if (activeTasks == 0 && taskQueue.empty()) {
                waitCV.notify_all();
            }
        }

        {
            lock_guard<mutex> lk(wts[id].mtx);
            wts[id].thunk = nullptr;
            wts[id].available = true;
        }
    }
}

void ThreadPool::wait() { // esoero para que los threads terminen
    unique_lock<mutex> lk(queueLock);
    waitCV.wait(lk, [this]() {
        return taskQueue.empty() && activeTasks == 0;
    });
}

ThreadPool::~ThreadPool() {
    wait();

    {
        lock_guard<mutex> lg(queueLock);
        done = true;
        dispatcherCV.notify_all();  // notificar con mutex bloqueado
    }

    dispatcherThread.join();

    for (auto& wt : wts) {
        wt.ready.signal();
        wt.ts.join();
    }
}
