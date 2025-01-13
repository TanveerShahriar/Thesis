#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>

// Define maximum recursion depth
const int MAX_DEPTH = 5;

// Forward declarations with varied return types and parameters
int f1(int a, int depth);
void f2(int depth);
double f3(double x, double y, int depth);
std::string f4(const std::string& s, int depth);
int f5(int a, int b, int depth);
void f6(int depth);
double f7(double x, int depth);
std::string f8(const std::string& s, int n, int depth);
int f9(int depth);
void f10(int depth);
double f11(double x, double y, double z, int depth);
std::string f12(const std::string& s, int depth);
int f13(int a, int depth);
void f14(int depth);
double f15(double x, int depth);
std::string f16(const std::string& s, const std::string& t, int depth);
int f17(int a, int b, int c, int depth);
void f18(int depth);
double f19(double x, int depth);
std::string f20(const std::string& s, int depth);
int f21(int a, int depth);
void f22(int depth);
double f23(double x, double y, int depth);
std::string f24(const std::string& s, int n, int depth);
int f25(int depth);
void f26(int depth);
double f27(double x, int depth);
std::string f28(const std::string& s, int depth);
int f29(int a, int b, int depth);
void f30(int depth);
double f31(double x, double y, double z, int depth);
std::string f32(const std::string& s, int depth);
int f33(int a, int depth);
void f34(int depth);
double f35(double x, int depth);
std::string f36(const std::string& s, const std::string& t, int depth);

// Function implementations

// Group 1: f1 - f6
int f1(int a, int depth) {
    std::cout << "f1 called with a = " << a << ", depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f1. Terminating recursion.\n";
        return a;
    }
    int result = a + 1;
    f2(depth + 1);
    f3(3.14, 2.71, depth + 1);
    return result;
}

void f2(int depth) {
    std::cout << "f2 called with depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f2. Terminating recursion.\n";
        return;
    }
    f4("Hello from f2", depth + 1);
    f5(10, 20, depth + 1);
}

double f3(double x, double y, int depth) {
    std::cout << "f3 called with x = " << x << ", y = " << y << ", depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f3. Terminating recursion.\n";
        return x + y;
    }
    double sum = x + y;
    f6(depth + 1);
    f7(5.5, depth + 1);
    return sum;
}

std::string f4(const std::string& s, int depth) {
    std::cout << "f4 called with s = " << s << ", depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f4. Terminating recursion.\n";
        return s;
    }
    f8(s, 3, depth + 1);
    f9(depth + 1);
    return s + " World";
}

int f5(int a, int b, int depth) {
    std::cout << "f5 called with a = " << a << ", b = " << b << ", depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f5. Terminating recursion.\n";
        return a + b;
    }
    int sum = a + b;
    f10(depth + 1);
    f11(1.1, 2.2, 3.3, depth + 1);
    return sum;
}

void f6(int depth) {
    std::cout << "f6 called with depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f6. Terminating recursion.\n";
        return;
    }
    f12("Function f12", depth + 1);
    f13(7, depth + 1);
}

// Group 2: f7 - f12
double f7(double x, int depth) {
    std::cout << "f7 called with x = " << x << ", depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f7. Terminating recursion.\n";
        return x * 2;
    }
    double result = x * 2;
    f14(depth + 1);
    f15(9.99, depth + 1);
    return result;
}

std::string f8(const std::string& s, int n, int depth) {
    std::cout << "f8 called with s = " << s << ", n = " << n << ", depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f8. Terminating recursion.\n";
        std::string repeated;
        for(int i = 0; i < n; ++i) {
            repeated += s;
        }
        return repeated;
    }
    std::string repeated;
    for(int i = 0; i < n; ++i) {
        repeated += s;
    }
    f16(s, "Programming", depth + 1);
    f17(2, 3, 4, depth + 1);
    return repeated;
}

int f9(int depth) {
    std::cout << "f9 called with depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f9. Terminating recursion.\n";
        return 42;
    }
    f18(depth + 1);
    f19(4.56, depth + 1);
    return 42;
}

void f10(int depth) {
    std::cout << "f10 called with depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f10. Terminating recursion.\n";
        return;
    }
    f20("Data from f10", depth + 1);
    f21(100, depth + 1);
}

double f11(double x, double y, double z, int depth) {
    std::cout << "f11 called with x = " << x << ", y = " << y << ", z = " << z << ", depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f11. Terminating recursion.\n";
        return (x + y + z) / 3.0;
    }
    double avg = (x + y + z) / 3.0;
    f22(depth + 1);
    f23(7.89, 1.23, depth + 1);
    return avg;
}

std::string f12(const std::string& s, int depth) {
    std::cout << "f12 called with s = " << s << ", depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f12. Terminating recursion.\n";
        return s + "!";
    }
    f24(s, 2, depth + 1);
    f25(depth + 1);
    return s + "!";
}

// Group 3: f13 - f18
int f13(int a, int depth) {
    std::cout << "f13 called with a = " << a << ", depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f13. Terminating recursion.\n";
        return a * a;
    }
    int square = a * a;
    f26(depth + 1);
    f27(3.14, depth + 1);
    return square;
}

void f14(int depth) {
    std::cout << "f14 called with depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f14. Terminating recursion.\n";
        return;
    }
    f28("Deep Call from f14", depth + 1);
    f29(5, 10, depth + 1);
}

double f15(double x, int depth) {
    std::cout << "f15 called with x = " << x << ", depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f15. Terminating recursion.\n";
        return x / 2.0;
    }
    double half = x / 2.0;
    f30(depth + 1);
    f31(2.2, 3.3, 4.4, depth + 1);
    return half;
}

std::string f16(const std::string& s, const std::string& t, int depth) {
    std::cout << "f16 called with s = " << s << ", t = " << t << ", depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f16. Terminating recursion.\n";
        return s + " " + t;
    }
    std::string combined = s + " " + t;
    f32(s, depth + 1);
    f33(8, depth + 1);
    return combined;
}

int f17(int a, int b, int c, int depth) {
    std::cout << "f17 called with a = " << a << ", b = " << b << ", c = " << c << ", depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f17. Terminating recursion.\n";
        return a * b * c;
    }
    int product = a * b * c;
    f34(depth + 1);
    f35(6.6, depth + 1);
    return product;
}

void f18(int depth) {
    std::cout << "f18 called with depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f18. Terminating recursion.\n";
        return;
    }
    f36("End", "Process", depth + 1);
    // f18 does not call any further functions to ensure f36 is the last in this path
}

// Group 4: f19 - f24
double f19(double x, int depth) {
    std::cout << "f19 called with x = " << x << ", depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f19. Terminating recursion.\n";
        return x;
    }
    if(x > 5.0) {
        std::cout << "x > 5.0, calling f1 with a = 10\n";
        return f1(10, depth + 1) * x;
    }
    f20("Data from f19", depth + 1);
    return x;
}

std::string f20(const std::string& s, int depth) {
    std::cout << "f20 called with s = " << s << ", depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f20. Terminating recursion.\n";
        return s + "!";
    }
    f21(100, depth + 1);
    f22(depth + 1);
    return s + "!";
}

int f21(int a, int depth) {
    std::cout << "f21 called with a = " << a << ", depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f21. Terminating recursion.\n";
        return a - 1;
    }
    f23(7.89, 1.23, depth + 1);
    f24("Sample Data", 2, depth + 1);
    return a - 1;
}

void f22(int depth) {
    std::cout << "f22 called with depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f22. Terminating recursion.\n";
        return;
    }
    f25(depth + 1);
    f26(depth + 1);
}

// Group 5: f25 - f30
double f23(double x, double y, int depth) {
    std::cout << "f23 called with x = " << x << ", y = " << y << ", depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f23. Terminating recursion.\n";
        return x - y;
    }
    double diff = x - y;
    f27(3.14, depth + 1);
    f28("Recursive Call from f23", depth + 1);
    return diff;
}

std::string f24(const std::string& s, int n, int depth) {
    std::cout << "f24 called with s = " << s << ", n = " << n << ", depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f24. Terminating recursion.\n";
        std::string repeated;
        for(int i = 0; i < n; ++i) {
            repeated += s;
        }
        return repeated;
    }
    std::string repeated;
    for(int i = 0; i < n; ++i) {
        repeated += s;
    }
    f29(5, 10, depth + 1);
    f30(depth + 1);
    return repeated;
}

int f25(int depth) {
    std::cout << "f25 called with depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f25. Terminating recursion.\n";
        return 0;
    }
    f31(2.2, 3.3, 4.4, depth + 1);
    f32("Chain Call", depth + 1);
    return 0;
}

void f26(int depth) {
    std::cout << "f26 called with depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f26. Terminating recursion.\n";
        return;
    }
    f33(8, depth + 1);
    f34(depth + 1);
}

// Group 6: f27 - f36
double f27(double x, int depth) {
    std::cout << "f27 called with x = " << x << ", depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f27. Terminating recursion.\n";
        return x;
    }
    if(x < 10.0) {
        std::cout << "x < 10.0, calling f3 with x = 1.1, y = 2.2\n";
        return f3(1.1, 2.2, depth + 1) + x;
    }
    f35(x, depth + 1);
    return x;
}

std::string f28(const std::string& s, int depth) {
    std::cout << "f28 called with s = " << s << ", depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f28. Terminating recursion.\n";
        return s;
    }
    f36("End", "Process", depth + 1);
    f1(15, depth + 1); // Indirect recursion
    return s + "!";
}

int f29(int a, int b, int depth) {
    std::cout << "f29 called with a = " << a << ", b = " << b << ", depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f29. Terminating recursion.\n";
        return a > b ? a : b;
    }
    int max = (a > b) ? a : b;
    f19(6.0, depth + 1);
    return max;
}

void f30(int depth) {
    std::cout << "f30 called with depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f30. Terminating recursion.\n";
        return;
    }
    f20("Loop Data", depth + 1);
    f22(depth + 1);
}

double f31(double x, double y, double z, int depth) {
    std::cout << "f31 called with x = " << x << ", y = " << y << ", z = " << z << ", depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f31. Terminating recursion.\n";
        return x + y + z;
    }
    double total = x + y + z;
    f32("Total from f31", depth + 1);
    f33(9, depth + 1);
    return total;
}

std::string f32(const std::string& s, int depth) {
    std::cout << "f32 called with s = " << s << ", depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f32. Terminating recursion.\n";
        return s + "!";
    }
    f34(depth + 1);
    f35(7.7, depth + 1);
    return s + "!";
}

int f33(int a, int depth) {
    std::cout << "f33 called with a = " << a << ", depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f33. Terminating recursion.\n";
        return a * 3;
    }
    f36("Final", "Stage", depth + 1);
    f1(a, depth + 1); // Indirect recursion
    return a * 3;
}

void f34(int depth) {
    std::cout << "f34 called with depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f34. Terminating recursion.\n";
        return;
    }
    f3(4.4, 5.5, depth + 1);
    f4("Deep Dive", depth + 1);
}

double f35(double x, int depth) {
    std::cout << "f35 called with x = " << x << ", depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f35. Terminating recursion.\n";
        return x * x;
    }
    double squared = x * x;
    f5(15, 25, depth + 1);
    f6(depth + 1);
    return squared;
}

std::string f36(const std::string& s, const std::string& t, int depth) {
    std::cout << "f36 called with s = " << s << ", t = " << t << ", depth = " << depth << "\n";
    if (depth >= MAX_DEPTH) {
        std::cout << "Max depth reached in f36. Terminating recursion.\n";
        return s + " " + t;
    }
    // Base case to prevent infinite recursion
    return s + " " + t;
}

// Modified Main function to start multiple call chains
int main() {
    std::srand(std::time(0)); // Seed for any random operations if needed
    std::cout << "Program started.\n";
    
    // Start multiple call chains to ensure all functions are called
    std::cout << "\n--- Starting Call Chain 1 ---\n";
    int initial1 = 5;
    int result1 = f1(initial1, 1); // Start recursion with depth 1
    std::cout << "Final result from f1: " << result1 << "\n";
    
    std::cout << "\n--- Starting Call Chain 2 ---\n";
    double initial2 = 10.0;
    double result2 = f3(initial2, 5.0, 1); // Start recursion with depth 1
    std::cout << "Final result from f3: " << result2 << "\n";
    
    std::cout << "\n--- Starting Call Chain 3 ---\n";
    std::string initial3 = "Start";
    std::string result3 = f4(initial3, 1); // Start recursion with depth 1
    std::cout << "Final result from f4: " << result3 << "\n";
    
    std::cout << "\n--- Starting Call Chain 4 ---\n";
    double initial4 = 7.0;
    double result4 = f7(initial4, 1); // Start recursion with depth 1
    std::cout << "Final result from f7: " << result4 << "\n";
    
    return 0;
}
