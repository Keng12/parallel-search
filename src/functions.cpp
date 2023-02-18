#include <array>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "vector.hpp"
#include "functions.hpp"

namespace kyc
{
    kyc::vector<std::string> setupData(std::string const &filename)
    {
        std::filesystem::path file{filename};
        kyc::vector<std::string> data{};
        if (!std::filesystem::exists(file))
        {
            std::cout << "Create input data" << std::endl;
            std::array<char, 26> alphabet{'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
            std::stringstream ss{};
            std::ofstream output{filename};

            for (int i = 0; i < 26; ++i)
            {
                for (int j = 0; j < 26; ++j)
                {
                    for (int k = 0; k < 26; ++k)
                    {
                        for (int l = 0; l < 26; ++l)
                        {
                            ss << alphabet.at(i) << alphabet.at(j) << alphabet.at(k) << alphabet.at(l);
                            data.push_back(ss.str());
                            output << ss.str() << '\n';
                            ss.str("");
                            ss.clear();
                        };
                    }
                }
            }
        }
        else
        {
            std::cout << "Read input data" << std::endl;
            std::ifstream input{filename};
            std::string line{};
            while (std::getline(input, line))
            {
                data.push_back(line);
            }
        }
        return data;
    }
    std::string getFilename(std::string const &dir, std::string const &basename, int &counter)
    {
        std::string filename{};
        std::filesystem::create_directory(std::filesystem::path{dir});
        while (true)
        {
            std::filesystem::path file{dir + "/" + basename + std::to_string(counter) + ".txt"};
            if (std::filesystem::exists(file))
            {
                ++counter;
            }
            else
            {
                filename = file.string();
                break;
            }
        }
        return filename;
    }
}
