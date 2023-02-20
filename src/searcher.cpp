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
                       bool &searchFinished, const int totalSize)
        : mData{inputVector}, mCV{mainCV}, mMainMutex{mainMutex},
          mSearchFinished{searchFinished}, mTotalSize{totalSize} {};

    void Searcher::start(int n)
    {
        assert(n > 0);
        mThreadpool.start(n);
        mWorkerThreads = mThreadpool.getNumberThreads();
    };

    kyc::vector<std::string> Searcher::search(const std::string &userInput)
    {
        kyc::vector<std::shared_ptr<kyc::vector<std::string>>> output{};
        searchJob(userInput, output);
        {
            std::unique_lock<std::shared_mutex> lock{mMainMutex};
            mCV.wait(lock, [this]
                     { return mSearchFinished; });
        }
        std::cout << "Finish job" << std::endl;
        kyc::vector<std::string> result{};
        for (int i = 0; i < mWorkerThreads; ++i)
        {
            result.append(*(output.at(i)));
        }
        return result;
    }

    void Searcher::searchJob(const std::string &userInput, kyc::vector<std::shared_ptr<kyc::vector<std::string>>> &output_ptr)
    {
        const int size = mData.getSize();
        mSearchFinished = false;
        std::shared_ptr<std::mutex> jobMutex = std::make_shared<std::mutex>();
        // Post jobs equal to number of working threads;
        mTotalCounter = 0;
        std::cout << "Total Size: " << mTotalSize << std::endl;

        for (int i = 0; i < mWorkerThreads; ++i)
        {
            std::shared_ptr<kyc::vector<std::string>> data = std::make_shared<kyc::vector<std::string>>(mData.at(i));
            std::shared_ptr<kyc::vector<std::string>> output = std::make_shared<kyc::vector<std::string>>();
            output_ptr.push_back(output);
            const auto job = [data, output, userInput, this]()
            {
                const int size = data->getSize();
                output->reserve(data->getSize());
                int index{};
                do
                {
                    if (index == size) // Counter has been reached
                    {
                        std::cout << "Final element " << index << std::endl;
                        mTotalCounter += index;
                        std::cout << "Total counter: " << mTotalCounter << std::endl;
                        if (mTotalCounter == mTotalSize)
                        {
                            notifyMainThread();
                        }
                        return;
                    }
                    std::string element{data->at(index)};
                    if (0 == element.rfind(userInput, 0))
                    {
                        output->push_back(std::move(element));
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
