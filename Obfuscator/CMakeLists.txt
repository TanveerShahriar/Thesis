cmake_minimum_required(VERSION 3.15)
project(Obfuscator)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Find LLVM and Clang
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

# Include LLVM and Clang headers
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})

# Link directories for Clang/LLVM
link_directories(${LLVM_LIBRARY_DIRS})

# Define macros
add_definitions(
    -D_FILE_OFFSET_BITS=64
    -D__STDC_CONSTANT_MACROS
    -D__STDC_FORMAT_MACROS
    -D__STDC_LIMIT_MACROS
    )

# Enable RTTI and exceptions
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-frtti -fexceptions)

# Add the source files
add_executable(Obfuscator
    obfuscator.cpp
)

# Add the include directory to the specific target's compile options
target_compile_options(Obfuscator PRIVATE -I/usr/lib/gcc/x86_64-linux-gnu/11/include/)

# Link Clang libraries
target_link_libraries(Obfuscator
    PRIVATE
    clangTooling
    clangAST
    clangFrontend
    clangBasic
    clangASTMatchers
)

# Add LLVM and Clang definitions
target_compile_definitions(Obfuscator
    PRIVATE
    ${LLVM_DEFINITIONS}
)

# Ensure the binary is built correctly
llvm_map_components_to_libnames(llvm_libs support core irreader)
target_link_libraries(Obfuscator PRIVATE ${llvm_libs})