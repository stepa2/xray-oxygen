#pragma once
/*
 * Basic C++11 based thread pool with per-thread job queues
 * Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>

class Thread 
{
private:
    bool destroying = false;
    std::thread worker;
    std::queue<std::function<void()>> jobQueue;
    std::mutex queueMutex;
    std::condition_variable condition;

    // Loop through all remaining jobs
    void queueLoop() 
	{
        while (true) 
		{
            std::function<void()> job;
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                condition.wait(lock, [this] { return !jobQueue.empty() || destroying; });
                if (destroying) {
                    break;
                }
                job = jobQueue.front();
            }

            job();

            {
                std::lock_guard<std::mutex> lock(queueMutex);
                jobQueue.pop();
                condition.notify_one();
            }
        }
    }

public:
    Thread() { worker = std::thread(&Thread::queueLoop, this); }

    ~Thread() 
	{
        if (worker.joinable())
		{
			wait();
			queueMutex.lock();
			destroying = true;
			condition.notify_one();
			queueMutex.unlock();
			worker.join();
        }
    }

    // Add a new job to the thread's queue
    void addJob(std::function<void()> function) 
	{
        std::lock_guard<std::mutex> lock(queueMutex);
        jobQueue.push(std::move(function));
        condition.notify_one();
    }

    // Wait until all work items have been finished
    void wait() 
	{
        std::unique_lock<std::mutex> lock(queueMutex);
        condition.wait(lock, [this]() { return jobQueue.empty(); });
    }
};

class ThreadPool {
public:
    std::vector<std::unique_ptr<Thread>> threads;

    void initialize() 
	{
        unsigned int num_threads = std::thread::hardware_concurrency();
        for (auto i = 0; i < num_threads; i++) {
            threads.push_back(std::make_unique<Thread>());
        }
    }

    // Wait until all threads have finished their work items
    void wait() 
	{
        for (auto& thread : threads) 
		{
            thread->wait();
        }
    }
};