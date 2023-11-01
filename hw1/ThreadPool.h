#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>

struct Task
{
    std::function<void()> func;
    Task(const std::function<void()>& func) : func(func) {}
};

class ThreadPool
{
public:
    ThreadPool(int numThreads);
    ~ThreadPool();

    // Function to add a task to the thread pool
    void AddTask(Task task);
    void WaitAll();

private:
    int numThreads;
    std::vector<std::thread> threads;
    int activeThreads;
    std::queue<Task> tasks;
    std::mutex taskMutex;
    std::mutex activeThreadsMutex;
    std::mutex coutMutex;
    std::condition_variable taskCondition;
    std::condition_variable noActiveThreadsCondition;
    bool stop;

    // Function that worker threads execute
    void WorkerThread();

    // Disable copy and assignment to ensure proper resource management
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
};
