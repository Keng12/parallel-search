#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <shared_mutex>

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
        std::shared_ptr<kyc::vector<std::string>> data = kyc::setupData("input.txt");
        kyc::Searcher searcher{nThreads};
        while (true)
        {
            std::shared_ptr<kyc::vector<std::string>> results = searcher.search(data);
            if (results->getSize() == 0)
            {   
                std::cout << "Exiting program" << std::endl;
                break;
            }
            else
            {
                std::stringstream ss{};
                for (int i = 0; i < results->getSize(); ++i)
                {
                    ss << results->at(i) << "\n";
                }
                std::ofstream out{filename};
                out << ss.str();
                ++counter;
                filename = basename + std::to_string(counter) + ".txt";
                // Increment search -> look for subset of input data -> swap input data with output data
                data.swap(results);
                std::cout << "Save results to: " << filename << "\n_____________________________________________________" << std::endl;
            }
        }
    }
    catch (std::exception const &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
