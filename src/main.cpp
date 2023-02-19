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

int main(int argc, char *argv[])
{
    try
    {
        const int nThreads = kyc::parseInput(argc, argv);
        std::string const basename{"output"};
        std::string const dir{"results"};
        auto [filename, counter] = kyc::getFilename(dir, basename);
        kyc::vector<std::string> data = kyc::setupData("input.txt");
        kyc::vector<kyc::vector<std::string>> chunkedData{};
        chunkedData.reserve(nThreads);
        const int divisor = data.getSize() / nThreads;
        const int remainder = data.getSize() % nThreads;
        for (int i = 0; i < nThreads; ++i)
        {
            kyc::vector<std::string> chunk{};
            int nElements{};
            if (i == nThreads - 1)
            {
                nElements = remainder;
            }
            else
            {
                nElements = divisor;
            }
            chunk = data.extract(i * divisor, nElements);
            chunkedData.push_back(chunk);
        }
        std::condition_variable_any condVar{};
        std::shared_mutex mutex{};
        bool searchFinished{};
        kyc::Searcher searcher{data, condVar, mutex, searchFinished};
        searcher.start(nThreads);
        while (true)
        {
            // During incrementel search the input will be received via an event handler
            std::string in{};
            std::cout << "Enter search string:" << std::endl;
            std::cin >> in;
            if ("0" != in)
            {
                std::cout << "Searching for: " << in << std::endl;
                std::shared_ptr<std::string> userInput = std::make_shared<std::string>(in);
                std::shared_ptr<kyc::vector<std::string>> output = std::make_shared<kyc::vector<std::string>>();
                auto const startTime = std::chrono::steady_clock::now();
                searcher.searchJob(output, userInput);
                {
                    std::unique_lock<std::shared_mutex> lock{mutex};
                    condVar.wait(lock, [&searchFinished]
                                 { return searchFinished; });
                }
                const std::chrono::duration<double> elapsedTime = std::chrono::steady_clock::now() - startTime;
                std::cout << "Search time: " << elapsedTime.count() << " seconds. Size: " << output->getSize() << std::endl;
                std::stringstream ss{};
                for (int i = 0; i < output->getSize(); ++i)
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
