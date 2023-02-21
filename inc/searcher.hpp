#ifndef SEARCHER_HPP
#define SEARCHER_HPP

#include <cmath>
#include <memory>
#include <mutex>

#include "threadpool.hpp"
#include "vector.hpp"

// Create lambda function to take indices as pass by copy/reference?
// Check if indices empty -> Yes: Quit
// No: Pop and continue
namespace kyc
{
  class Searcher
  {
    kyc::Threadpool mThreadpool{};
    int const mWorkerThreads{};
    std::condition_variable mCV{};
    std::mutex mMutex{};
    bool mSearchFinished{};
    std::atomic<int> mTotalCounter{};
    int mTotalSize{};

  public:
    Searcher(const int nThreads);
    void postSearchJob(std::shared_ptr<kyc::vector<std::string>> inputData, const std::string &userInput, std::shared_ptr<kyc::vector<std::string>> output_ptr);
    std::shared_ptr<kyc::vector<std::string>> search(std::shared_ptr<kyc::vector<std::string>> inputData, const std::string &userInput);
    void stop();
    void notifyMainThread();
  };
} // namespace kyc
#endif
