#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

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

namespace kyc
{
    class Threadpool
    {
        std::mutex mMutex{};
        std::vector<std::thread> mThreads{};
        std::deque<std::function<void()>> mJobQueue{};
        std::atomic_bool mShutdown{};
        std::condition_variable mCondVar{}; // Allows threads to wait on new jobs or termination
        int mWorkerThreads{};

    public:
        bool idle();
        void start(int nWorkerThreads);
        void threadLoop();
        void postJob(const std::function<void()> &job);
        void stop();
        int getNumberThreads() const;
    };

} // namespace v

#endif
