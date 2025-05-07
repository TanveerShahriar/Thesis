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


class FunctionInfo:
    def __init__(self, function_name, function_body, params=""):
        self.function_name: str = function_name
        self.function_body: str = function_body
        self.time_complexity: str = None
        self.total_statement: int = 0
        self.function_name_with_params: str = params

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
        return self.function_name_with_params


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
    print(f"{ConsoleColors.OKCYAN}Total functions: {len(functions)}{ConsoleColors.ENDC}")


saveAsCppFile(functions)
