#ifndef CPP_FUNCTIONS_H
#define CPP_FUNCTIONS_H

#include <unordered_map>
#include <string>

// Inline global hashmap mapping function names to their time complexities.
inline const std::unordered_map<std::string, std::string> cppFunctionsMap = {
    {"funcD", "(1)"},
    {"funcB", "4"},
    {"funcE", "3"},
    {"funcC", "(1 + (4 * (1 + 1))) + (1 + (5 * (1 + 1)))"},
    {"funcA", "3"},
};

#endif // CPP_FUNCTIONS_H
