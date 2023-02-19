#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "functions.hpp"
#include "searcher.hpp"
#include "vector.hpp"

int main()
{
    try
    {
        // int counter{};
        std::string basename{"output"};
        std::string const dir{"results"};
        auto [filename, counter] = kyc::getFilename(dir, basename);
        kyc::vector<std::string> data = kyc::setupData("input.txt");
        std::condition_variable condVar{};
        std::mutex mutex{};
        bool searchFinished{};
        bool cancelSearch{};
        kyc::Searcher searcher{data, condVar, mutex, searchFinished, cancelSearch};
        std::cout << "Setup searcher" << std::endl;
        searcher.start(std::thread::hardware_concurrency() - 1);
        std::cout << "Setup searcher finished" << std::endl;
        while (true)
        {
            // During incrementel search the input will be received via an event handler
            std::string in{};
            std::cout << "Enter search string:" << std::endl;
            std::cin >> in;
            if ("0" != in)
            {
                std::cout << "Searching for: " << in << std::endl;
                std::shared_ptr<std::string> userInput =
                    std::make_shared<std::string>(in);
                std::shared_ptr<kyc::vector<std::string>> output =
                    std::make_shared<kyc::vector<std::string>>();

                auto startTime = std::chrono::steady_clock::now();
                searcher.searchJob(output, userInput);
                {
                    std::unique_lock<std::mutex> lock{mutex};
                    condVar.wait(lock, [&searchFinished, &cancelSearch]
                                 { return searchFinished || cancelSearch; });
                    // Event handler will set the cancelSearch flag if the job queue is not empty and notify main thread
                    if (cancelSearch)
                    {
                        searcher.clearQueue();
                        searchFinished = true; // Set to true so all remaining threads will return to wait
                        cancelSearch = false;
                        continue;
                    }
                }
                const std::chrono::duration<double> elapsedTime = std::chrono::steady_clock::now() - startTime;
                std::cout << "Search time: " << elapsedTime.count() << " seconds. Size: " << output->size() << std::endl;
                std::stringstream ss{};
                for (int i = 0; i < output->size(); ++i)
                {
                    ss << output->at(i) << "\n";
                }
                std::ofstream out{filename};
                out << ss.str();
                ++counter;
                filename = dir + "/" + basename + std::to_string(counter) + ".txt";
                std::cout << "Wrote search results of " << in << " in: " << filename << std::endl;
            }
            else
            {
                std::cout << "Exiting" << std::endl;
                break;
            }
        }
        searcher.stop();
    }
    catch (std::exception const &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
