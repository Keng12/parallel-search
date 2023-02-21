#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <memory>
#include <numeric>
#include <mutex>
#include <thread>
#include <vector>

#include "vector.hpp"

namespace kyc {
class Threadpool {
  std::mutex mMutex{};
  std::vector<std::thread> mThreads{};
  std::vector<std::function<void()>> mJobQueue{};
  std::atomic_bool mShutdown{};
  std::condition_variable mCondVar{}; // Allows threads to wait on new jobs or termination
  int mWorkerThreads{};
  bool mStarted{};

public:
  Threadpool();
  Threadpool(int const nWorkerThreads);
  ~Threadpool();
  void start(int const nWorkerThreads);
  void threadLoop();
  void postJob(const std::function<void()> &job);
  void stop();
  int getNumberThreads() const;
};

} // namespace kyc

#endif
