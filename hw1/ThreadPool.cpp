#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "ThreadPool.h"

ThreadPool::ThreadPool(int numThreads) : numThreads(numThreads), stop(false), activeThreads(0)
{
    for (int i = 0; i < numThreads; ++i)
    {
        threads.emplace_back(&ThreadPool::WorkerThread, this);
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(taskMutex);
        stop = true;
        taskCondition.notify_all();
    }

    for (std::thread& thread : threads)
    {
        thread.join();
    }
}

void ThreadPool::AddTask(Task& task)
{
    {
        std::unique_lock<std::mutex> lock(taskMutex);
        tasks.push(task);
    }
    taskCondition.notify_one();
}

void ThreadPool::WorkerThread()
{
    while (true)
    {
        Task task;

        {
            std::unique_lock<std::mutex> lock(taskMutex);

            taskCondition.wait(lock, [this] { return stop || !tasks.empty(); });

            if (stop && tasks.empty()) {
                return;
            }

            task = tasks.front();
            tasks.pop();
        }

        // Increment the activeThreads count safely
        {
            std::unique_lock<std::mutex> lock(activeThreadsMutex);
            activeThreads++;
        }

        // Execute the task
        task.func(task.arg);

        // Decrement the activeThreads count safely
        {
            std::unique_lock<std::mutex> lock(activeThreadsMutex);
            activeThreads--;
            if (activeThreads == 0) {
                noActiveThreadsCondition.notify_one();
            }
        }
    }
}

void ThreadPool::WaitAll()
{
    std::unique_lock<std::mutex> lock(taskMutex);
    std::unique_lock<std::mutex> activeLock(activeThreadsMutex);
    noActiveThreadsCondition.wait(activeLock, [this] { return tasks.empty() && activeThreads == 0; });
}
