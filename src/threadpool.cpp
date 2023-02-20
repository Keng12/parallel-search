#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <iostream>
#include <memory>
#include <numeric>
#include <shared_mutex>
#include <thread>
#include <vector>

#include "threadpool.hpp"
#include "vector.hpp"

namespace kyc
{
  void Threadpool::start(int nWorkerThreads)
  {
    assert(nWorkerThreads > 0);
    mWorkerThreads = nWorkerThreads;
    std::cout << "Threadpool: No. of threads: " << mWorkerThreads << std::endl;
    mThreads.reserve(mWorkerThreads);
    mJobQueue.reserve(mWorkerThreads);
    for (int i = 0; i < mWorkerThreads; ++i)
    {
      mThreads.emplace_back(&Threadpool::threadLoop, this);
    }
  }

  void Threadpool::threadLoop()
  {
    while (true)
    {
      std::function<void()> job;
      {
        std::unique_lock<std::shared_mutex> lock{mMutex};
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
  }

  void Threadpool::postJob(const std::function<void()> &job)
  {
    {
      std::unique_lock<std::shared_mutex> lock(mMutex);
      mJobQueue.push_back(job);
    }
    mCondVar.notify_one();
  }

  void Threadpool::stop()
  {
    {
      std::unique_lock<std::shared_mutex> lock(mMutex);
      mShutdown = true;
    }
    mCondVar.notify_all();
    for (std::thread &thread : mThreads)
    {
      thread.join();
    }
  }

  int Threadpool::getNumberThreads() const { return mWorkerThreads; }

  bool Threadpool::idle()
  {
    bool idle{};
    {
      std::shared_lock<std::shared_mutex> lock(mMutex);
      idle = mJobQueue.empty();
    }
    return idle;
  }
}; // namespace kyc
