#ifndef CPP_FUNCTIONS_H
#define CPP_FUNCTIONS_H

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <utility>

inline const std::unordered_map<std::string, std::pair<std::string, int>> cppFunctionsMap = {
    {"funcD_ii", std::make_pair("4", 26)},
    {"funcB", std::make_pair("2 + 1 + 2 + 1", 10)},
    {"funcE_ii", std::make_pair("3", 11)},
    {"funcC", std::make_pair("2 + 3", 10)},
    {"funcA", std::make_pair("4", 10)},
};

inline const std::unordered_set<std::string> cppFunctionNamesSet = {
    "funcB",
    "funcC",
    "funcE",
    "funcD",
    "funcA",
};

#endif
