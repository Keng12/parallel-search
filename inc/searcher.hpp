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
namespace kyc {
class Searcher {
  kyc::Threadpool mThreadpool{};
  int mWorkerThreads{};
  kyc::vector<std::string> &mData;
  std::condition_variable &mCV;
  std::mutex &mMainMutex;
  bool &mSearchFinished;

public:
  Searcher(kyc::vector<std::string> &inputVector,
           std::condition_variable &mainCV, std::mutex &mainMutex,
           bool &searchFinished);

  void start(int n);
  void searchJob(std::shared_ptr<kyc::vector<std::string>> outputVector,
                 std::shared_ptr<std::string> userInput);
  void stop();
  bool idle();
  void notifyMainThread();
  void clearQueue();
};
} // namespace kyc
#endif
