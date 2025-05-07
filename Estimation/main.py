import os
import time
import clang.cindex
from time_complexity_analyzer import analyze_time_complexity

MAX_ATTEMPTS = 5


class ConsoleColors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


class Parameter:
    def __init__(self, name, type):
        self.name: str = name
        self.type: str = type

    def __repr__(self):
        return f"{self.type} {self.name}"


class FunctionInfo:
    def __init__(self, function_name, function_body, params=""):
        self.function_name: str = function_name
        self.function_body: str = function_body
        self.time_complexity: str = None
        self.total_statement: int = 0
        self.function_name_with_params: str = params
        self.params: list[Parameter] = []
        self.return_type: str = None

    def setTimeComplexity(self, time_complexity):
        self.time_complexity = time_complexity

    def getTimeComplexity(self):
        return self.time_complexity

    def setTotalStatements(self, count):
        self.total_statement = count

    def getTotalStatements(self):
        return self.total_statement

    def setFunctionNameWithParams(self, params):
        self.function_name_with_params = params

    def getFunctionNameWithParams(self):
        return self.function_name_with_params

    def __repr__(self):
        return f"{self.function_name_with_params}({', '.join(map(str, self.params))}) -> {self.return_type}"


def extract_functions_from_source(file_path):
    index = clang.cindex.Index.create()

    _, ext = os.path.splitext(file_path.lower())
    args = []
    if ext == '.c':
        args = ['-std=c11', '-x', 'c']
    elif ext in ['.cpp', '.cc', '.cxx']:
        args = ['-std=c++11', '-x', 'c++']
    elif ext == '.h':
        args = ['-std=c++11', '-x', 'c++-header']
    else:
        args = ['-std=c++11']

    try:
        tu = index.parse(file_path, args=args)
    except clang.cindex.TranslationUnitLoadError as e:
        print(f"Error parsing {file_path}: {e}")
        return []

    functions = []
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            source_lines = f.readlines()
    except Exception as e:
        print(f"Error reading file {file_path}: {e}")
        return functions

    def get_source_code(start, end):
        if start.line == end.line:
            return source_lines[start.line - 1][start.column - 1:end.column - 1]
        else:
            lines = source_lines[start.line - 1:end.line]
            lines[0] = lines[0][start.column - 1:]
            lines[-1] = lines[-1][:end.column - 1]
            return ''.join(lines)

    def visit(node):
        is_user_file = node.location.file and (
            os.path.abspath(
                node.location.file.name) == os.path.abspath(file_path)
        )
        is_top_level_function = (
            node.kind == clang.cindex.CursorKind.FUNCTION_DECL and
            node.semantic_parent.kind == clang.cindex.CursorKind.TRANSLATION_UNIT
        )
        if is_top_level_function and is_user_file and node.is_definition() and node.spelling != "main":
            extent = node.extent
            function_code = get_source_code(extent.start, extent.end)
            func_info = FunctionInfo(node.spelling, function_code.strip())

            for param in node.get_arguments():
                param_name = param.spelling
                param_type = param.type.spelling
                func_info.params.append(Parameter(param_name, param_type))

            return_type = node.result_type.spelling
            func_info.return_type = None if return_type == "void" else return_type

            param_types = []
            for param in node.get_arguments():
                param_type = param.type.spelling.split()[-1]
                param_types.append(param_type[0])
            params_suffix = ''.join(param_types)
            func_info.setFunctionNameWithParams(
                f"{node.spelling}{f'_{params_suffix}' if params_suffix else ''}"
            )

            try:
                total_statements = count_statements(node)
            except Exception as e:
                print(f"Error counting statements in {node.spelling}: {e}")
                total_statements = 0
            func_info.setTotalStatements(total_statements)

            functions.append(func_info)

        for child in node.get_children():
            visit(child)

    visit(tu.cursor)
    return functions


def count_statements(node):
    """
    Recursively count statements in the function node by traversing child nodes
    that are of statement types.
    """
    statement_count = 0

    if node.kind in [
        clang.cindex.CursorKind.BINARY_OPERATOR,
        clang.cindex.CursorKind.CALL_EXPR,
        clang.cindex.CursorKind.DECL_REF_EXPR,
        clang.cindex.CursorKind.IF_STMT,
        clang.cindex.CursorKind.FOR_STMT,
        clang.cindex.CursorKind.WHILE_STMT,
        clang.cindex.CursorKind.SWITCH_STMT,
        clang.cindex.CursorKind.RETURN_STMT
    ]:
        statement_count += 1

    for child in node.get_children():
        statement_count += count_statements(child)

    return statement_count


def total_cpp_file_count(source_folder):
    """Count the number of .c, .cpp, and header files in the source_folder."""
    count = 0
    for root, dirs, files in os.walk(source_folder):
        for file in files:
            if file.endswith(('.cpp', '.cc', '.cxx', '.c', '.h')):
                count += 1
    return count


def extract_all_functions_from_project(source_folder):
    """Recursively walk through the source_folder and extract functions from .c, .cpp, and header files."""
    all_functions = []
    total_files = total_cpp_file_count(source_folder)
    cpp_files_count = 0
    for root, dirs, files in os.walk(source_folder):
        for file in files:
            if file.endswith(('.cpp', '.cc', '.cxx', '.c', '.h')):
                cpp_files_count += 1
                file_path = os.path.join(root, file)
                currentTime = time.time()
                print(
                    f"{ConsoleColors.OKCYAN}[{cpp_files_count}/{total_files}][{time.strftime('%H:%M:%S', time.localtime(currentTime))}] Analyzing file: {file_path}{ConsoleColors.ENDC}", end="")
                functions = extract_functions_from_source(file_path)
                print(
                    f" {ConsoleColors.OKGREEN}====== Found [{len(functions)}] ====== Total [{len(all_functions) + len(functions)}]{ConsoleColors.ENDC}")
                all_functions.extend(functions)
    return all_functions


source_folder = "../Input"
functions = extract_all_functions_from_project(source_folder)
limit = False

total_elapsed_time = 0

for index, func in enumerate(functions):
    isSuccess = False
    count = 0
    time_complexity = None
    startingTime = time.time()
    report = ""
    while not isSuccess:
        print(
            f"{ConsoleColors.OKBLUE}Generating Complexity Attempt-[{count+1}/{MAX_ATTEMPTS}]: {func.function_name}{ConsoleColors.ENDC}")
        isSuccess, time_complexity = analyze_time_complexity(
            str(func.function_body), report=report)
        count += 1
        if not isSuccess:
            report += f"Attempt {count}: {time_complexity}\n"
        if count >= MAX_ATTEMPTS:
            count = 0
            isSuccess = False
            break
    endingTime = time.time()
    elapsedTime = endingTime - startingTime
    total_elapsed_time += elapsedTime
    avg_time_per_function = total_elapsed_time / (index + 1)
    remaining_functions = len(functions) - (index + 1)
    approx_eta = avg_time_per_function * remaining_functions

    print(f"{ConsoleColors.OKCYAN}Generation time: {elapsedTime:.2f} seconds{ConsoleColors.ENDC}")
    if isSuccess:
        func.setTimeComplexity(time_complexity)
        print(
            f"{ConsoleColors.OKGREEN}Time complexity: {func.getTimeComplexity()}{ConsoleColors.ENDC}")
    else:
        print(f"{ConsoleColors.FAIL}Failed to analyze time complexity, using default value.{ConsoleColors.ENDC}")
        func.setTimeComplexity(str(func.getTotalStatements()))
    print(f"{ConsoleColors.OKCYAN}Total Statements: {func.getTotalStatements()}{ConsoleColors.ENDC}")
    eta_hours, eta_remainder = divmod(approx_eta, 3600)
    eta_minutes, eta_seconds = divmod(eta_remainder, 60)
    print(
        f"{ConsoleColors.HEADER}======================================= [{index+1}/{len(functions)}] {(index+1) / len(functions) * 100:.2f}% | ETA: {int(eta_hours):02}:{int(eta_minutes):02}:{int(eta_seconds):02} ======================================={ConsoleColors.ENDC}")
    if limit and index == 2:
        break


def saveAsCppFile(functions):
    print(f"{ConsoleColors.OKCYAN}Saving C++ header file...{ConsoleColors.ENDC}")
    header_content = '''\
#ifndef CPP_FUNCTIONS_H
#define CPP_FUNCTIONS_H

#include <unordered_map>
#include <unordered_set>
#include <string>

inline const std::unordered_map<std::string, std::string> cppFunctionsMap = {
'''
    print(f"{ConsoleColors.OKCYAN}Generating HashMap...{ConsoleColors.ENDC}")
    unique_functions = {}
    function_names_set = set()
    for func in functions:
        key = func.getFunctionNameWithParams()
        if key not in unique_functions:
            complexity = func.getTimeComplexity() if func.getTimeComplexity(
            ) is not None else func.getTotalStatements()
            unique_functions[key] = complexity
        function_names_set.add(func.function_name)

    for key, complexity in unique_functions.items():
        header_content += f'    {{"{key}", "{complexity}"}},\n'

    header_content += '''\
};

inline const std::unordered_set<std::string> cppFunctionNamesSet = {
'''
    print(f"{ConsoleColors.OKCYAN}Generating HashSet...{ConsoleColors.ENDC}")
    for name in function_names_set:
        header_content += f'    "{name}",\n'

    header_content += '};\n\n'
    header_content += '#endif\n'

    output_folder = "../Obfuscator"
    os.makedirs(output_folder, exist_ok=True)
    output_file_path = os.path.join(output_folder, "cpp_functions.h")
    with open(output_file_path, "w", encoding="utf-8") as header_file:
        header_file.write(header_content)

    print(f"{ConsoleColors.OKGREEN}C++ header file saved successfully at {output_file_path}{ConsoleColors.ENDC}")


saveAsCppFile(functions)


def saveObfuscatorHppFile(functions):
    print(f"{ConsoleColors.OKCYAN}Saving Obfuscator header file...{ConsoleColors.ENDC}")
    header_content = '''\
#ifndef OBFUSCATOR_H
#define OBFUSCATOR_H

#include <queue>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <random>
#include <thread>

using namespace std;

constexpr int OBFUSCATION_THREADS = 2;

enum FunctionID
{
'''
    for index, func in enumerate(functions):
        header_content += f'    {func.getFunctionNameWithParams().upper()},\n'
    header_content += '''\
};
'''
    for func in functions:
        header_content += '''
struct'''
        header_content += f' {func.getFunctionNameWithParams()}_values\n'
        header_content += '''\
{
'''
        for param in func.params:
            header_content += f'    {param.type} {param.name};\n'
        if func.return_type:
            header_content += f'    {func.return_type} {func.getFunctionNameWithParams()}_return;\n'
        header_content += f'    bool {func.getFunctionNameWithParams()}_done;\n'
        header_content += '''\
};

'''

    for func in functions:
        header_content += f'''\
extern queue<int> {func.getFunctionNameWithParams()}_params_index_pool;
'''
    header_content += f'''\

'''
    for func in functions:
        header_content += f'''\
extern vector<{func.getFunctionNameWithParams()}_values> {func.getFunctionNameWithParams()}_params;
'''
    header_content += f'''\

extern thread threads[OBFUSCATION_THREADS];
extern queue<pair<int, int>> queues[OBFUSCATION_THREADS];
extern mutex mutexes[OBFUSCATION_THREADS];
extern condition_variable conditions[OBFUSCATION_THREADS];

extern bool stopThreads;
extern mutex stopMutex;

extern atomic<int> g_inFlightTasks;
extern condition_variable g_allTasksDoneCV;
extern mutex g_allTasksDoneMtx;

extern std::atomic<int> *vec;

void initialize();
void exit();
void taskFinished();
int getBalancedRandomIndex();
void pushToThread(int funcId, int line_no, int param_index);
void execute(int thread_idx);
void threadFunction(int thread_idx);

'''
    for func in functions:
        header_content += f'''\
void {func.getFunctionNameWithParams()}(int thread_idx, int param_index);
'''

    header_content += '\n#endif\n'
    output_folder = "../Input"
    os.makedirs(output_folder, exist_ok=True)
    output_file_path = os.path.join(output_folder, "obfuscator.hpp")
    with open(output_file_path, "w", encoding="utf-8") as header_file:
        header_file.write(header_content)

    print(f"{ConsoleColors.OKGREEN}Obfuscator header file saved successfully at {output_file_path}{ConsoleColors.ENDC}")


saveObfuscatorHppFile(functions)


def saveObfuscatorCppFile(functions):
    print(f"{ConsoleColors.OKCYAN}Saving Obfuscator cpp file...{ConsoleColors.ENDC}")
    header_content = '''\
#include <algorithm>
#include "obfuscator.hpp"

thread threads[OBFUSCATION_THREADS];

queue<pair<int, int>> queues[OBFUSCATION_THREADS];
mutex mutexes[OBFUSCATION_THREADS];
condition_variable conditions[OBFUSCATION_THREADS];

bool stopThreads = false;
mutex stopMutex;

atomic<int> g_inFlightTasks{0};
condition_variable g_allTasksDoneCV;
mutex g_allTasksDoneMtx;

'''
    for func in functions:
        header_content += f'''\
queue<int> {func.getFunctionNameWithParams()}_params_index_pool;
'''
    header_content += f'''\

'''
    for func in functions:
        header_content += f'''\
vector<{func.getFunctionNameWithParams()}_values> {func.getFunctionNameWithParams()}_params;
'''
    header_content += '''\

std::atomic<int> *vec;

std::random_device rd;
std::mt19937 rng(rd());

void initialize()
{
    vec = new std::atomic<int>[OBFUSCATION_THREADS];
    for (int i = 0; i < OBFUSCATION_THREADS; i++)
    {
        vec[i].store(0);
    }

    for (int i = 0; i < OBFUSCATION_THREADS; i++)
    {
        threads[i] = thread(threadFunction, i);
    }
}

void exit()
{
    unique_lock<mutex> lock(g_allTasksDoneMtx);
    g_allTasksDoneCV.wait(lock, []
                          { return g_inFlightTasks.load() == 0; });

    {
        lock_guard<mutex> stopLock(stopMutex);
        stopThreads = true;
    }

    for (int i = 0; i < OBFUSCATION_THREADS; i++)
    {
        conditions[i].notify_all();
        threads[i].join();
    }
}

int getBalancedRandomIndex()
{
    double sum = 0;
    std::vector<int> values(OBFUSCATION_THREADS);

    for (int i = 0; i < OBFUSCATION_THREADS; i++)
    {
        values[i] = vec[i].load();
        sum += values[i];
    }

    double avg = sum / OBFUSCATION_THREADS;
    double threshold = avg * 0.8;

    std::vector<int> candidateIndices;

    for (int i = 0; i < OBFUSCATION_THREADS; i++)
    {
        if (values[i] <= threshold)
        {
            candidateIndices.push_back(i);
        }
    }

    if (candidateIndices.empty())
    {
        std::vector<int> sortedValues = values;
        std::sort(sortedValues.begin(), sortedValues.end());
        int median = sortedValues[OBFUSCATION_THREADS / 2];

        for (int i = 0; i < OBFUSCATION_THREADS; i++)
        {
            if (values[i] <= median)
            {
                candidateIndices.push_back(i);
            }
        }
    }

    std::uniform_int_distribution<int> dist(0, candidateIndices.size() - 1);
    return candidateIndices[dist(rng)];
}

void pushToThread(int funcId, int line_no, int param_index)
{
    int thread_idx = getBalancedRandomIndex();
    {
        lock_guard<mutex> lock(mutexes[thread_idx]);
        queues[thread_idx].emplace(funcId, param_index);
        vec[thread_idx].fetch_add(line_no);
        g_inFlightTasks++;
    }
    conditions[thread_idx].notify_one();
}

void taskFinished()
{
    int remaining = --g_inFlightTasks;
    if (remaining == 0)
    {
        unique_lock<mutex> lock(g_allTasksDoneMtx);
        g_allTasksDoneCV.notify_all();
    }
}

void execute(int thread_idx)
{
    if (queues[thread_idx].empty())
        return;

    pair<int, int> func_info;
    {
        lock_guard<mutex> lock(mutexes[thread_idx]);
        func_info = queues[thread_idx].front();
        queues[thread_idx].pop();
    }

    switch (func_info.first)
    {
'''
    for func in functions:
        header_content += f'    case {func.getFunctionNameWithParams().upper()}:\n'
        header_content += f'        {func.getFunctionNameWithParams()}(thread_idx, func_info.second);\n'
        header_content += '        break;\n'
    header_content += '''\
    }

    taskFinished();
}

void threadFunction(int thread_idx)
{
    while (true)
    {
        {
            unique_lock<mutex> lock(mutexes[thread_idx]);
            conditions[thread_idx].wait(lock, [&]
                                        { return !queues[thread_idx].empty() || stopThreads; });
        }

        if (stopThreads && queues[thread_idx].empty())
            break;
        execute(thread_idx);
    }
}
'''

    header_content += '\n'
    output_folder = "../Input"
    os.makedirs(output_folder, exist_ok=True)
    output_file_path = os.path.join(output_folder, "obfuscator.cpp")
    with open(output_file_path, "w", encoding="utf-8") as header_file:
        header_file.write(header_content)

    print(f"{ConsoleColors.OKGREEN}Obfuscator cpp file saved successfully at {output_file_path}{ConsoleColors.ENDC}")


saveObfuscatorCppFile(functions)
