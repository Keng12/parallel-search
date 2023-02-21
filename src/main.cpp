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
        std::pair<std::string, int> filedata = kyc::getFilename(basename);
        std::string filename = filedata.first;
        int counter = filedata.second;
        kyc::vector<std::string> data = kyc::setupData("input.txt");
        std::cout << "Data: " << data.getSize() << std::endl;
        kyc::vector<kyc::vector<std::string>> chunkedData = kyc::splitData(data, nThreads);
        std::cout << "Finish split data (main)" << std::endl;
        kyc::Searcher searcher{chunkedData, data.getSize()};
        searcher.start(nThreads);
        while (true)
        {
            // During incremental search the input will be received via an event handler
            std::string input{};
            std::cout << "Enter search string:" << std::endl;
            std::cin >> input;
            if ("0" != input)
            {
                std::cout << "Searching for: " << input << std::endl;
                auto const startTime = std::chrono::steady_clock::now();
                std::shared_ptr<kyc::vector<std::string>> const results = searcher.search(input);
                const std::chrono::duration<double> elapsedTime = std::chrono::steady_clock::now() - startTime;
                std::cout << "Search time: " << elapsedTime.count() << " seconds. Count: " << results->getSize() << std::endl;
                std::stringstream ss{};
                for (int i = 0; i < results->getSize(); ++i)
                {
                    ss << results->at(i) << "\n";
                }
                std::ofstream out{filename};
                out << ss.str();
                ++counter;
                filename = basename + std::to_string(counter) + ".txt";
                std::cout << "Wrote search results of " << input << " in: " << filename << std::endl;
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
