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

    kyc::vector<std::string> Searcher::search(const std::string &userInput)
    {
        std::shared_ptr<kyc::vector<kyc::vector<std::string>>> output_ptr = std::make_shared<kyc::vector<kyc::vector<std::string>>>();
        searchJob(userInput, output_ptr);
        {
            std::unique_lock<std::shared_mutex> lock{mMainMutex};
            mCV.wait(lock, [this]
                     { return mSearchFinished; });
        }
        std::cout << "Finish job" <<std::endl;
        kyc::vector<std::string> output{};
        for (int i = 0; i < mWorkerThreads; ++i)
        {
            output.reserve(output_ptr->at(i).getSize());
            output.append(output_ptr->at(i));
        }
        std::cout << "Appended output" <<std::endl;
        return output;
    }

    void Searcher::searchJob(const std::string &userInput, std::shared_ptr<kyc::vector<kyc::vector<std::string>>> output_ptr)
    {
        const int size = mData.getSize();
        mSearchFinished = false;
        std::shared_ptr<std::mutex> jobMutex = std::make_shared<std::mutex>();
        // Post jobs equal to number of working threads;
        for (int i = 0; i < mWorkerThreads; ++i)
        {
            std::shared_ptr<kyc::vector<std::string>> data = std::make_shared<kyc::vector<std::string>>(mData.at(i));
            std::shared_ptr<kyc::vector<std::string>> outputVector = std::make_shared<kyc::vector<std::string>>();
            const auto job = [data, outputVector, userInput, this]()
            {
                const int size = data->getSize();
                std::cout << "Data size: " << data->getSize() << "output size: " << outputVector->getSize()  << std::endl;
                outputVector->reserve(data->getSize());
                int index{};
                do
                {
                    if (index == size) // Counter has been reached
                    {
                        if (mThreadpool.idle())
                        {
                            notifyMainThread();
                        }
                        return;
                    }
                    std::string element{data->at(index)};
                    if (0 == element.rfind(element, 0))
                    {
                        outputVector->push_back(std::move(element));
                    }
                    ++index;
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
