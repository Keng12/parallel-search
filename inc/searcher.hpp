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
    int mWorkerThreads{};
    kyc::vector<kyc::vector<std::string>> &mData;
    std::condition_variable mCV{};
    std::mutex mMutex{};
    bool mSearchFinished{};
    std::atomic<int> mTotalCounter{};
    const int mTotalSize{};

  public:
    Searcher(kyc::vector<kyc::vector<std::string>> &inputVector, const int totalSize, const int nThreads);
    void searchJob(const std::string &userInput, std::shared_ptr<kyc::vector<std::string>> output_ptr);
    std::shared_ptr<kyc::vector<std::string>> search(const std::string &userInput);
    void stop();
    void notifyMainThread();
  };
} // namespace kyc
#endif
