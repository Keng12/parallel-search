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
    Searcher::Searcher(int nThreads, bool &searchCanceled) : mWorkerThreads{std::move(nThreads)}, mThreadpool{mWorkerThreads}, mSearchCanceled{searchCanceled}
    {
        assert(mWorkerThreads > 0);
    };

    std::shared_ptr<kyc::vector<std::string>> Searcher::search(std::shared_ptr<kyc::vector<std::string>> inputData, const std::string &userInput)
    {
        std::shared_ptr<kyc::vector<std::string>> output = std::make_shared<kyc::vector<std::string>>();
        {
            // Wait for input event to get user input
            // Event handler -> construct input string based on incoming characters
            // Retrieve input here, wait if no input available
            // Set boolean to false after retrieving input
            std::lock_guard<std::shared_timed_mutex> lock{mMutex};
            mSearchCanceled = false;
            // In demo: Use user input provided by std::cin instead
        }
        mSearchFinished = false;
        std::cout << "Input data size: " << inputData->getSize() << std::endl;
        if (inputData->getSize() > 0)
        {
            postSearchJob(inputData, userInput, output);
            bool canceled{};
            {
                // Event handler: 1. Get input, 2. If search not cancelled -> cancel search, notify main thread
                std::unique_lock<std::shared_timed_mutex> lock{mMutex};
                mCV.wait(lock, [this]
                         { return mSearchFinished || mSearchCanceled; }); // Wait for input event or until search is finished
                mSearchFinished = true;                                   // In case search got canceled -> Do not update output data
                canceled = mSearchCanceled;
            }
            if (canceled) // Outside unique_lock, output not modified anymore after setting mSearchFinished
            {
                std::cout << "Search canceled" << std::endl;
                output.swap(inputData);
            }
            else
            {
                std::cout << "Search completed" << std::endl;
            }
        }
        else
        {
            std::cout << "User input empty, no search done" << std::endl;
        }
        return output;
    }

    void Searcher::postSearchJob(std::shared_ptr<kyc::vector<std::string>> inputData, const std::string &userInput, std::shared_ptr<kyc::vector<std::string>> output_ptr)
    {
        std::shared_ptr<std::mutex> jobMutex = std::make_shared<std::mutex>();
        const int totalSize = inputData->getSize();
        const int chunkSize = totalSize / mWorkerThreads;
        const int remainder = totalSize % mWorkerThreads;
        std::shared_ptr<int> totalCounter = std::make_shared<int>();
        int nJobs{};
        if (chunkSize == 0)
        {
            nJobs = remainder;
        }
        else
        {
            nJobs = mWorkerThreads;
        }
        for (int i = 0; i < nJobs; ++i)
        {
            const auto job = [inputData, output_ptr, userInput, i, jobMutex, chunkSize, remainder, totalSize, totalCounter, this]()
            {
                kyc::vector<std::string> tmpData{};
                if (0 == i)
                {
                    tmpData = inputData->extract(0, chunkSize + remainder);
                }
                else
                {
                    tmpData = inputData->extract(i * chunkSize + remainder, chunkSize);
                }
                const int size = tmpData.getSize();
                kyc::vector<std::string> tmpOutput{};
                tmpOutput.reserve(size);
                int index{};
                while (!getSearchCanceled())
                {
                    if (size == index) // Counter has been reached
                    {
                        int tmpCounter{};
                        {
                            std::lock_guard<std::mutex> const lock{*jobMutex};
                            *totalCounter += index;
                            tmpCounter = *totalCounter;
                        }
                        if (tmpOutput.getSize() > 0 && !getSearchFinished())
                        {
                            std::lock_guard<std::mutex> const lock{*jobMutex};
                            output_ptr->append(tmpOutput);
                        }
                        if (totalSize == tmpCounter && !getSearchFinished())
                        {
                            notifyMainThread();
                        }
                        return;
                    }
                    std::string element{tmpData.at(index)};
                    if (0 == element.rfind(userInput, 0))
                    {
                        tmpOutput.push_back(element);
                    }
                    ++index;
                }
            };
            mThreadpool.postJob(job);
        }
    };

    void Searcher::stop() { mThreadpool.stop(); };
    void Searcher::notifyMainThread()
    {
        {
            std::lock_guard<std::shared_timed_mutex> const lock{mMutex};
            mSearchFinished = true;
        }
        mCV.notify_one();
        return;
    }

    bool Searcher::getSearchFinished()
    {
        bool tmpBool{};
        {
            std::shared_lock<std::shared_timed_mutex> const lock{mMutex};
            tmpBool = mSearchFinished;
        }
        return tmpBool;
    }

    bool Searcher::getSearchCanceled()
    {
        bool tmpBool{};
        {
            std::shared_lock<std::shared_timed_mutex> const lock{mMutex};
            tmpBool = mSearchCanceled;
        }
        return tmpBool;
    }
}; // namespace kyc
