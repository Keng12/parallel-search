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
    Searcher::Searcher(kyc::vector<kyc::vector<std::string>> &inputVector, const int totalSize, const int nThreads)
        : mData{inputVector}, mTotalSize{totalSize}, mWorkerThreads{nThreads}
    {
        assert(nThreads > 0);
        mThreadpool.start(nThreads);
    };

    std::shared_ptr<kyc::vector<std::string>> Searcher::search(const std::string &userInput)
    {
        mSearchFinished = false;
        mTotalCounter = 0;
        std::shared_ptr<kyc::vector<std::string>> output = std::make_shared<kyc::vector<std::string>>();
        searchJob(userInput, output);
        {
            std::unique_lock<std::mutex> lock{mMutex};
            mCV.wait(lock, [this]
                     { return mSearchFinished; });
        }
        std::cout << "Finished search" << std::endl;
        return output;
    }

    void Searcher::searchJob(const std::string &userInput, std::shared_ptr<kyc::vector<std::string>> output_ptr)
    {
        std::shared_ptr<std::mutex> jobMutex = std::make_shared<std::mutex>();
        // Post jobs equal to number of working threads;
        for (int i = 0; i < mWorkerThreads; ++i)
        {
            const auto job = [output_ptr, userInput, i, jobMutex, this]()
            {
                kyc::vector<std::string> const tmpData{mData.at(i)};
                const int size = tmpData.getSize();
                kyc::vector<std::string> tmpOutput{};
                tmpOutput.reserve(size);
                int index{};

                do
                {
                    if (size == index) // Counter has been reached
                    {

                        mTotalCounter += index;
                        {
                            std::lock_guard<std::mutex> const lock{*jobMutex};
                            output_ptr->append(tmpOutput);
                        }
                        if (mTotalSize == mTotalCounter)
                        {
                            notifyMainThread();
                        }
                        return;
                    }
                    std::string element{tmpOutput.at(index)};
                    if (0 == element.rfind(userInput, 0))
                    {
                        tmpOutput.push_back(std::move(element));
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
