#ifndef SEARCHER_HPP
#define SEARCHER_HPP

#include <cmath>
#include <memory>
#include <mutex>
#include <shared_mutex>

#include "threadpool.hpp"
#include "vector.hpp"

// Create lambda function to take indices as pass by copy/reference?
// Check if indices empty -> Yes: Quit
// No: Pop and continue
namespace kyc
{
  class Searcher
  {
    int const mWorkerThreads;
    bool &mSearchCanceled;
    kyc::Threadpool mThreadpool;
    std::condition_variable_any mCV{};
    std::shared_timed_mutex mMutex{};
    bool mSearchFinished{};
    bool getSearchCanceled();
    bool getSearchFinished();
    std::string mUserInput{};
    bool mEventHandler{};

  public:
    Searcher(const int nThreads, bool &searchCanceled);
    void postSearchJob(std::shared_ptr<kyc::vector<std::string>> inputData, std::shared_ptr<kyc::vector<std::string>> output_ptr);
    std::shared_ptr<kyc::vector<std::string>> search(std::shared_ptr<kyc::vector<std::string>> inputData);
    void stop();
    void notifyMainThread();
  };
} // namespace kyc
#endif
