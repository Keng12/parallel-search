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
    Searcher::Searcher(int nThreads) : mWorkerThreads{std::move(nThreads)}, mThreadpool{mWorkerThreads}
    {
        assert(mWorkerThreads > 0);
    };

    std::shared_ptr<kyc::vector<std::string>> Searcher::search(std::shared_ptr<kyc::vector<std::string>> inputData, const std::string &userInput)
    {
        mSearchFinished = false;
        mTotalCounter = 0;
        std::shared_ptr<kyc::vector<std::string>> output = std::make_shared<kyc::vector<std::string>>();
        postSearchJob(inputData, userInput, output);
        {
            std::unique_lock<std::mutex> lock{mMutex};
            mCV.wait(lock, [this]
                     { return mSearchFinished; });
        }
        std::cout << "Finished search" << std::endl;
        return output;
    }

    void Searcher::postSearchJob(std::shared_ptr<kyc::vector<std::string>> inputData, const std::string &userInput, std::shared_ptr<kyc::vector<std::string>> output_ptr)
    {
        std::shared_ptr<std::mutex> jobMutex = std::make_shared<std::mutex>();
        const int totalSize = inputData->getSize();
        const int chunkSize = totalSize / mWorkerThreads;
        const int remainder = totalSize % mWorkerThreads;
        // Post jobs equal to number of working threads;
        for (int i = 0; i < mWorkerThreads; ++i)
        {
            const auto job = [inputData, output_ptr, userInput, i, jobMutex, chunkSize, remainder, totalSize, this]()
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
                do
                {
                    if (size == index) // Counter has been reached
                    {
                        mTotalCounter += index;
                        if (tmpOutput.getSize() > 0)
                        {
                            std::lock_guard<std::mutex> const lock{*jobMutex};
                            output_ptr->append(tmpOutput);
                        }
                        if (totalSize == mTotalCounter)
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
                } while (true);
            };
            mThreadpool.postJob(job);
        }
    };

    void Searcher::stop() { mThreadpool.stop(); };
    void Searcher::notifyMainThread()
    {
        {
            std::lock_guard<std::mutex> const lock{mMutex};
            mSearchFinished = true;
        }
        mCV.notify_one();
        return;
    }
}; // namespace kyc
