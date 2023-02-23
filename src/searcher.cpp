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
    Searcher::Searcher(int nThreads) : mWorkerThreads{std::move(nThreads)}, mThreadpool{mWorkerThreads}, mEventHandlerThread{mCV, mEventMutex, mSearchInterrupted}
    {
        assert(mWorkerThreads > 0);
    };

    std::shared_ptr<kyc::vector<std::string>> Searcher::search(std::shared_ptr<kyc::vector<std::string>> inputData)
    {
        std::shared_ptr<kyc::vector<std::string>> output = std::make_shared<kyc::vector<std::string>>();
        mSearchFinished = false;
        {
            std::unique_lock<std::mutex> lock{mEventMutex};
            std::string const bufferedInput = mEventHandlerThread.getBufferedString();
            if (bufferedInput == mUserInput)
            {
                std::cout << "Enter character; press '0' to exit" << std::endl;
                mCV.wait(lock, [this]
                         { return mUserInput != mEventHandlerThread.getBufferedString(); });
                mUserInput = mEventHandlerThread.getBufferedString();
            }
            else
            {
                mUserInput = bufferedInput;
            }
            mSearchInterrupted = false;
        }
        if (inputData->getSize() > 0 && !mUserInput.empty() && mUserInput.back() != '0' && mUserInput.length() < 5)
        {
            std::cout << "Searching for: " << mUserInput << std::endl;
            bool interrupted{};
            auto const startTime = std::chrono::steady_clock::now();
            postSearchJob(inputData, output);
            {
                std::unique_lock<std::mutex> lock{mSearchMutex};
                mCV.wait(lock, [this]
                         { return mSearchFinished; }); // Wait for input event or until search is finished
                interrupted = mSearchInterrupted;
            }
            const std::chrono::duration<double> elapsedTime = std::chrono::steady_clock::now() - startTime;
            if (interrupted)
            {
                std::cout << "Search interrupted" << std::endl;
                // Outside unique_lock, output not modified anymore after setting mSearchInterrupted
                output.swap(inputData); // Revert output to input if cancelled
            }
            else
            {
                std::cout << "Search time: " << elapsedTime.count() << " seconds. Count: " << output->getSize() << " out of " << inputData->getSize() << std::endl;
            }
        }
        else if (!mUserInput.empty() && mUserInput.back() == '0')
        {
            std::cout << "'0' entered. Exiting program'" << std::endl;
        }
        else if (mUserInput.length() >= 5)
        {
            std::cout << "More than four characters were received ('" << mUserInput << "'). Exiting program." << std::endl;
        }
        else if (mUserInput.empty())
        {
            std::cout << "User input empty, no search done" << std::endl;
            output.swap(inputData); // Revert output to input if cancelled
        }
        else if (inputData->getSize() == 0)
        {
            std::cout << "Input data empty, no search done" << std::endl;
        }
        return output;
    }

    void Searcher::postSearchJob(std::shared_ptr<kyc::vector<std::string>> inputData, std::shared_ptr<kyc::vector<std::string>> output_ptr)
    {
        std::shared_ptr<std::mutex> jobMutex = std::make_shared<std::mutex>();
        const int totalSize = inputData->getSize();
        int chunkSize = totalSize / mWorkerThreads;
        int remainder = totalSize % mWorkerThreads;
        std::shared_ptr<int> totalCounter = std::make_shared<int>();
        int nJobs{};
        if (chunkSize == 0)
        {
            chunkSize = 1;
            nJobs = remainder;
            remainder = 0;
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
               // using namespace std::chrono_literals;
              //  std::this_thread::sleep_for(5s);
                while (true)
                {
                    if (size == index)
                    {

                        // Lock mutex only if condition is fulfilled
                        if (tmpOutput.getSize() > 0)
                        {
                            // Append to output vector if necessary
                            std::lock_guard<std::mutex> const lock{*jobMutex};
                            output_ptr->append(tmpOutput);
                        }
                        // Always lock mutex
                        int tmpCounter{};
                        {
                            std::lock_guard<std::mutex> const lock{*jobMutex};
                            *totalCounter += index;
                            tmpCounter = *totalCounter;
                        }
                        if (totalSize == tmpCounter)
                        {
                            // Notify main thread if search finished
                            notifyMainThread();
                        }
                        return;
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
    void Searcher::notifyMainThread()
    {
        {
            std::lock_guard<std::mutex> const lock{mSearchMutex};
            mSearchFinished = true;
        }
        mCV.notify_one();
        return;
    }

    bool Searcher::getsearchInterrupted()
    {
        bool tmpBool{};
        {
            std::lock_guard<std::mutex> const lock{mEventMutex};
            tmpBool = mSearchInterrupted;
        }
        return tmpBool;
    }

    std::string Searcher::getInputString() const
    {
        return mUserInput;
    }

}; // namespace kyc
