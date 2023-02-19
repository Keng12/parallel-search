#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

#include "vector.hpp"
#include <array>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <tuple>

namespace kyc {
kyc::vector<std::string> setupData(std::string const &filename);
std::tuple<std::string, int> getFilename(std::string const &dir,
                                         std::string const &basename);
int parseInput(const int argc, char **argv);
kyc::vector<std::string> searchData();
} // namespace kyc
#endif
