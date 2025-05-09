#ifndef CPP_FUNCTIONS_H
#define CPP_FUNCTIONS_H

#include <unordered_map>
#include <unordered_set>
#include <string>

inline const std::unordered_map<std::string, std::string> cppFunctionsMap = {
    {"funcD_ii", "4"},
    {"funcB", "2 + 1 + 2 + 1"},
    {"funcE_ii", "3"},
    {"funcC", "2 + 3"},
    {"funcA", "4"},
};

inline const std::unordered_set<std::string> cppFunctionNamesSet = {
    "funcA",
    "funcE",
    "funcC",
    "funcB",
    "funcD",
};

#endif
