#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

#include "vector.hpp"
#include <array>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <tuple>

namespace kyc
{
    std::shared_ptr<kyc::vector<std::string>> setupData(std::string const &filename);
    std::pair<std::string, int> getFilename(std::string const &basename);
    int parseInput(const int argc, char **argv);
} // namespace kyc
#endif
