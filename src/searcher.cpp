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

    std::shared_ptr<kyc::vector<std::string>> Searcher::search(std::shared_ptr<kyc::vector<std::string>> inputData)
    {
        std::shared_ptr<kyc::vector<std::string>> output = std::make_shared<kyc::vector<std::string>>();
        mUserInput.clear();
        if (mEventHandler)
        {
            std::unique_lock<std::shared_timed_mutex> lock{mMutex};
            /*
            - eventHandler.bufferedString -> Buffered string to search
            - Construct input string by appending incoming characters to buffered string
            - Event handler: if (!mSearchCanceled) {mSearchCanceled = true; notify main thread}
            if (eventHandler.bufferedString == mUserInput){
                waitForNewEvent(); //
            }
            mUserInput = eventHandler.bufferedString;
            mSearchCanceled = false;
            */
        }
        else
        {
            std::cout << "Enter search string:" << std::endl;
            std::cin >> mUserInput;
            std::cout << "Searching for: " << mUserInput << std::endl;
        }
        mSearchFinished = false;
        std::cout << "Input data size: " << inputData->getSize() << std::endl;
        if (inputData->getSize() > 0)
        {
            auto const startTime = std::chrono::steady_clock::now();
            postSearchJob(inputData, output);
            bool canceled{};
            {
                // Event handler: 1. Get input, 2. If search not cancelled -> cancel search, notify main thread
                std::unique_lock<std::shared_timed_mutex> lock{mMutex};
                mCV.wait(lock, [this]
                         { return mSearchFinished || mSearchCanceled; }); // Wait for input event or until search is finished
                mSearchFinished = true;                                   // In case search got canceled -> Do not update output data
                canceled = mSearchCanceled;
            }
            const std::chrono::duration<double> elapsedTime = std::chrono::steady_clock::now() - startTime;
            std::cout << "Search time: " << elapsedTime.count() << " seconds. Count: " << output->getSize() << std::endl;
            if (canceled) // Outside unique_lock, output not modified anymore after setting mSearchCanceled
            {
                std::cout << "Search canceled" << std::endl;
                output.swap(inputData); // Revert output to input
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

    void Searcher::postSearchJob(std::shared_ptr<kyc::vector<std::string>> inputData, std::shared_ptr<kyc::vector<std::string>> output_ptr)
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
            const auto job = [inputData, output_ptr, i, jobMutex, chunkSize, remainder, totalSize, totalCounter, this]()
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
                    if (size == index)
                    {
                        if (!getSearchCanceled())
                        {
                            // If event handler is used: return after each check
                            if (tmpOutput.getSize() > 0 && !getSearchCanceled())
                            {
                                // Append to output vector if necessary
                                std::lock_guard<std::mutex> const lock{*jobMutex};
                                output_ptr->append(tmpOutput);
                            }
                            int tmpCounter{};
                            if (!getSearchCanceled())
                            {
                                // Increment and check total counter or return if canceled
                                std::lock_guard<std::mutex> const lock{*jobMutex};
                                *totalCounter += index;
                                tmpCounter = *totalCounter;
                            }
                            if (totalSize == tmpCounter && !getSearchCanceled())
                            {
                                // Notify main thread if search finished
                                notifyMainThread();
                            }
                            return;
                        }
                        else
                        {
                            return;
                        }
                    }
                    std::string element{tmpData.at(index)};
                    if (0 == element.rfind(mUserInput, 0))
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
