#include <array>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>

#include "functions.hpp"
#include "vector.hpp"

namespace kyc
{
  bool fileExists(const std::string &filename)
  {
    std::ifstream f{filename};
    return f.good();
  }

  kyc::vector<std::string> setupData(std::string const &filename)
  {
    kyc::vector<std::string> data{};
    data.reserve(std::pow(26, 4));
    if (!fileExists(filename))
    {
      std::cout << "Create input data" << std::endl;
      std::array<char, 26> alphabet{'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
                                    'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
                                    'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
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
              ss << alphabet.at(i) << alphabet.at(j) << alphabet.at(k)
                 << alphabet.at(l);
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
    std::cout << "Returning: " << data.getSize() << std::endl;
    return data;
  }

  std::pair<std::string, int> getFilename(std::string const &basename)
  {
    std::string filename{};
    int counter{};
    while (true)
    {
      filename = std::string{basename + std::to_string(counter) +
                                 ".txt"};
      if (fileExists(filename))
      {
        ++counter;
      }
      else
      {
        break;
      }
    }
    return std::pair<std::string, int>{filename, counter};
  }

  int parseInput(const int argc, char **argv)
  {
    int nThreads{};
    if (argc > 2)
    {
      throw std::invalid_argument{"Please enter number of threads or no argument"};
    }
    else if (argc == 1)
    {
      nThreads = 1;
    }
    else
    {
      std::string arg{argv[1]};
      try
      {
        std::size_t pos{};
        nThreads = std::stoi(argv[1], &pos);
        if (pos < arg.size())
        {
          std::cerr << "Trailing characters after number: " << arg << '\n';
        }
      }
      catch (std::invalid_argument const &ex)
      {
        std::cerr << "Invalid number: " << arg << '\n';
        std::rethrow_exception(std::current_exception());
      }
      catch (std::out_of_range const &ex)
      {
        std::cerr << "Number out of range: " << arg << '\n';
        std::rethrow_exception(std::current_exception());
      }
      if (nThreads <= 0)
      {
        throw std::invalid_argument{"Number of threads specified " + arg + " is <= 0"};
      }
    }
    return nThreads;
  }

  kyc::vector<kyc::vector<std::string>> splitData(kyc::vector<std::string> data, int nChunks)
  {
    assert(nChunks > 0);
    // Check if nThreads > data size()
    const int chunkSize = data.getSize() / nChunks;
    kyc::vector<kyc::vector<std::string>> chunkedData{};
    chunkedData.reserve(nChunks);
    const int remainder = data.getSize() - chunkSize * nChunks;
    for (int i = 0; i < nChunks; ++i)
    {
      kyc::vector<std::string> chunk{data.extract(i * chunkSize + remainder, chunkSize)};
      if (0 == i)
      {
        chunk = data.extract(0, chunkSize + remainder);
      }
      else
      {
        chunk = data.extract(i * chunkSize + remainder, chunkSize);
      }
      std::cout << "Chunk size: " << chunk.getSize() << std::endl;
      chunkedData.push_back(chunk);
    }
    std::cout << "No of chunks: " << chunkedData.getSize() << std::endl;
    return chunkedData;
  }
} // namespace kyc
