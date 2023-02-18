#include <memory>
#include <shared_mutex>
#include <deque>
#include <iostream>
#include <thread>
#include <algorithm>
#include <numeric>
#include <atomic>
#include <condition_variable>
#include <vector>
#include <functional>

#include "vector.hpp"
#include "threadpool.hpp"

namespace kyc
{
    void Threadpool::start(int nWorkerThreads)
    {
        mWorkerThreads = nWorkerThreads;
        std::cout << "Threadpool: no threads: " << mWorkerThreads << std::endl;
        mThreads.reserve(mWorkerThreads);
        for (int i = 0; i < mWorkerThreads; ++i)
        {
            mThreads.emplace_back(&Threadpool::threadLoop, this);
        }
    };

    void Threadpool::threadLoop()
    {
        while (true)
        {
            std::function<void()> job;
            {
                std::unique_lock<std::mutex> lock(mMutex);
                mCondVar.wait(lock, [this]
                              { return !mJobQueue.empty() || mShutdown; });
                if (mShutdown)
                {
                    return;
                }
                job = mJobQueue.back();
                mJobQueue.pop_back();
            }
            job();
        }
    };
    void Threadpool::postJob(const std::function<void()> &job)
    {
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mJobQueue.push_front(job);
        }
        mCondVar.notify_one();
    };

    void Threadpool::stop()
    {
        {
            std::unique_lock<std::mutex> lock(mMutex);
            mShutdown = true;
        }
        mCondVar.notify_all();
        for (std::thread &thread : mThreads)
        {
            thread.join();
        }
        mThreads.clear();
    };

    int Threadpool::getNumberThreads() const
    {
        return mWorkerThreads;
    }

    bool Threadpool::idle()
    {
        bool idle{};
        {
            std::lock_guard<std::mutex> lock(mMutex);
            idle = mJobQueue.empty();
        }
        return idle;
    }
};
