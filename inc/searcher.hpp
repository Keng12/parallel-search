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
  kyc::vector<kyc::vector<std::string>> &mData;
  std::condition_variable_any &mCV;
  std::shared_mutex &mMainMutex;
  bool &mSearchFinished;

  bool getSearchFinished();

public:
  Searcher(kyc::vector<kyc::vector<std::string>> &inputVector,
           std::condition_variable_any &mainCV, std::shared_mutex &mainMutex,
           bool &searchFinished);

  void start(int n);
  void searchJob(std::shared_ptr<kyc::vector<std::string>> outputVector,
                 std::shared_ptr<std::string> userInput);
  void stop();
  void notifyMainThread();
};
} // namespace kyc
#endif
