#include <cmath>
#include <memory>
#include <mutex>

#include "searcher.hpp"
#include "threadpool.hpp"
#include "vector.hpp"
// Create lambda function to take indices as pass by copy/reference?
// Check if indices empty -> Yes: Quit
// No: Pop and continue
namespace kyc
{
    Searcher::Searcher(kyc::vector<kyc::vector<std::string>> &inputVector,
                       std::condition_variable_any &mainCV, std::shared_mutex &mainMutex,
                       bool &searchFinished)
        : mData{inputVector}, mCV{mainCV}, mMainMutex{mainMutex},
          mSearchFinished{searchFinished} {};

    void Searcher::start(int n)
    {
        assert(n > 0);
        mThreadpool.start(n);
        mWorkerThreads = mThreadpool.getNumberThreads();
    };

    void Searcher::searchJob(std::shared_ptr<std::string> userInput)
    {
        const int size = mData.getSize();
        mSearchFinished = false;
        std::shared_ptr<int> counter = std::make_shared<int>(0);
        std::shared_ptr<std::mutex> jobMutex = std::make_shared<std::mutex>();
        // Post jobs equal to number of working threads;
        for (int i = 0; i < mWorkerThreads; ++i)
        {
            std::shared_ptr<kyc::vector<std::string>> data = std::make_shared<kyc::vector<std::string>>(mData.at(i));
            std::shared_ptr<kyc::vector<std::string>> outputVector = std::make_shared<kyc::vector<std::string>>();
            const auto job = [data, outputVector, userInput]()
            {
                outputVector->reserve(data->getSize());
                do
                {
                    int index{};
                    bool finished{};
                    {
                        std::lock_guard<std::mutex> lock{*jobMutex};
                        if (size == *counter) // Counter has been reached
                        {
                            // If counter has been reached but jobs are still running -> Possible that elements are processing -> do not notify yet
                            if (mThreadpool.idle() && !getSearchFinished())
                            {
                                // Final job and counter has been reached -> notify main thread
                                // Necessary if job processing final element is not the final job in queue
                                notifyMainThread();
                            }
                            return;
                        }
                        else
                        {
                            index = *counter;
                            ++(*counter);
                            if (size == *counter) // Processing last element -> Notify main thread afterwards
                            {
                                finished = true; // Set local boolean
                            }
                        }
                    }
                    std::string element{mData.at(index)};
                    if (0 == element.rfind(*userInput, 0))
                    {
                        outputVector->push_back(element);
                    }
                    // Check if BOTH finished and last job
                    // Possible: Finished but not final job (e.g. if previous jobs still
                    // processing) or final job but not finished yet (e.g. during startup)
                    if (finished && mThreadpool.idle())
                    {
                        notifyMainThread(); // Set boolean for main thread AFTER pushing back element
                    }
                    // In case final job is not final element to be processed -> Still cancel while loop
                } while (!getSearchFinished());
            };
            mThreadpool.postJob(job);
        }
    };

    void Searcher::stop() { mThreadpool.stop(); };
    void Searcher::notifyMainThread()
    {
        {
            std::unique_lock<std::shared_mutex> lock{mMainMutex};
            mSearchFinished = true;
        }
        mCV.notify_one();
        return;
    }

    bool Searcher::getSearchFinished()
    {
        bool searchFinished{};
        {
            std::shared_lock<std::shared_mutex> lock{mMainMutex};
            searchFinished = mSearchFinished;
        }
        return searchFinished;
    }
}; // namespace kyc
