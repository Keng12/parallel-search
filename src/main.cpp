#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include <fstream>
#include <thread>

#include "vector.hpp"
#include "searcher.hpp"
#include "functions.hpp"

int main()
{
    try
    {
        int counter{};
        std::string basename{"output"};
        std::string const dir{"results"};
        std::string filename = kyc::getFilename(dir, basename, counter);
        kyc::vector<std::string> data = kyc::setupData("input.txt");
        std::condition_variable condVar{};
        std::mutex mutex{};
        bool searchFlag{};

        kyc::Searcher searcher{data, condVar, mutex, searchFlag};
        std::cout << "Setup searcher" << std::endl;
        searcher.start(std::thread::hardware_concurrency() - 1);
        std::cout << "Setup searcher finished" << std::endl;
        while (true)
        {
            std::string in{};
            std::cout << "Enter search string:" << std::endl;
            std::cin >> in;
            if ("0" != in)
            {
                std::cout << "Searching for: " << in << std::endl;
                std::shared_ptr<std::string> userInput = std::make_shared<std::string>(in);
                std::shared_ptr<kyc::vector<std::string>>
                    output = std::make_shared<kyc::vector<std::string>>();

                auto startTime = std::chrono::steady_clock::now();
                searcher.searchJob(output, userInput);
                {
                    std::unique_lock<std::mutex> lock{mutex};
                    condVar.wait(lock, [&searchFlag]
                                 { return searchFlag; });
                }
                std::chrono::duration<double> elapsedTime = std::chrono::steady_clock::now() - startTime;
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
