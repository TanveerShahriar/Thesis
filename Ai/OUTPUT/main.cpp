#include <iostream>
#include <iomanip>
#include "cpp_functions.h"

int main()
{
    std::cout << std::left << std::setw(25) << "Function Name" << "Time Complexity" << std::endl;
    std::cout << std::string(50, '-') << std::endl;

    for (const auto &pair : cppFunctionsMap)
    {
        std::cout << std::left << std::setw(25) << pair.first << pair.second << std::endl;
    }

    return 0;
}
