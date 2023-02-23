#ifndef SEARCHER_HPP
#define SEARCHER_HPP

#include <cmath>
#include <memory>
#include <mutex>
#include <shared_mutex>

#include "threadpool.hpp"
#include "vector.hpp"
#include "eventhandler.hpp"
// Create lambda function to take indices as pass by copy/reference?
// Check if indices empty -> Yes: Quit
// No: Pop and continue
namespace kyc
{
  class Searcher
  {
    int const mWorkerThreads;
    bool msearchInterrupted{};
    kyc::Threadpool mThreadpool;
    std::condition_variable mCV{};
    std::mutex mSearchMutex{};
    std::mutex mEventMutex{};
    bool mSearchFinished{};
    std::string mUserInput{};
    kyc::EventHandler mEventHandlerThread;

  public:
    Searcher(const int nThreads);
    void postSearchJob(std::shared_ptr<kyc::vector<std::string>> inputData, std::shared_ptr<kyc::vector<std::string>> output_ptr);
    std::shared_ptr<kyc::vector<std::string>> search(std::shared_ptr<kyc::vector<std::string>> inputData);
    void notifyMainThread();
    std::string getInputString() const;
    bool getsearchInterrupted();
  };
} // namespace kyc
#endif
