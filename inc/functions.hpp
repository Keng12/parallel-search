#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

#include <array>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "vector.hpp"

namespace kyc
{
    kyc::vector<std::string> setupData(std::string const &filename);
    std::string getFilename(std::string const &dir, std::string const &basename, int &counter);
}
#endif
