import json
from langchain_community.llms import LlamaCpp
from langchain.output_parsers import StructuredOutputParser, ResponseSchema

model_path = "DeepSeek-Coder-V2-Lite-Instruct-Q4_K_M.gguf"

llm = LlamaCpp(
    model_path=model_path,
    n_ctx=131072,
    n_gpu_layers=20,
    verbose=False,
    temperature=0.0
)

response_schemas = [
    ResponseSchema(
        name="time_complexity_calculation",
        description="A valid C++ expression that estimates the number of basic operations required to run the given function. The expression should be concrete and evaluable in C++ code, representing the total number of operations based on the input size and structure of the function. Avoid using Big O notation or symbolic variables unless they are part of the function's parameters. Ensure the expression is syntactically correct and can be directly used in a C++ program."
    ),
]

output_parser = StructuredOutputParser.from_response_schemas(response_schemas)
format_instructions = output_parser.get_format_instructions()

examples = """
// Example 1:
C++ function:
int Function(const std::vector<int>& givenArr) {
    int sum = 0;
    for (int num : givenArr) {
        sum += num;
    }
    return sum;
}
// Expected time complexity statement: "givenArr.size()"
// Explanation: The function has O(n) time complexity.

// Example 2:
C++ function:
void Function() {
    int n = 1000;
    int counter = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            counter++;
        }
    }
    std::cout << "Total iterations: " << counter << std::endl;
}
// Expected time complexity statement: "1000*1000"
// Explanation: The function has O(n^2) time complexity.

// Example 3:
C++ function:
void Function(const int arr[], int n) {
    long long sum = 0; 
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) {
                sum += arr[i] + arr[j] + arr[k];
            }
        }
    }
    std::cout << "Resulting sum: " << sum << std::endl;
}
// Expected time complexity statement: "n*n*n"
// Explanation: The function has O(n^3) time complexity.

// Example 4:
C++ function:
void Function() {
    int y = 0;
    std::cout << "Hello World" << std::endl;
}
// Expected time complexity statement: "2"
// Explanation: The function has O(2) time complexity.
"""


def analyze_time_complexity(cpp_function: str, report: str = "") -> tuple:
    prompt = f"""
    You are an AI that generates a valid C++ expression representing the estimated time needed to run a given function.
    The expression should be such that when used in the code:
        int timeNeedToRun = (expression);
    it evaluates to an integer representing the approximate number of basic operations, assuming each operation takes 1 unit of time.
    You do not replay the time complexity in Big O notation, but rather as a concrete expression.
    The expression should be a valid C++ expression that can be evaluated to an integer.
    If you can not determine the time complexity, return the number of statements in the function body.


    Below are several examples demonstrating the expected output:
    {examples}

    Now, analyze the following C++ function and provide a valid C++ expression following these format instructions exactly:
    {format_instructions}

    {f"Previously you tried this with the same provided cpp function and it was not valid expression, an validation ai generate this reason why it is not valid response: {report}. So This time make sure the output is a valid cpp expression" if report else ""}
    
    C++ function to analyze:
    {cpp_function}

    """

    try:
        response = llm(prompt)
        # print("Response from LLM:", response)
    except Exception as e:
        print("Error generating response:", e)
        return False, str(e)

    try:
        txt = response.split("}")
        txt = response.split("}")[-2].strip()
        txt = txt.split("{")[-1].strip()
        txt = f'```json\n{{{txt}}}\n```'
        parsed_output = output_parser.parse(txt)
        return True, parsed_output["time_complexity_calculation"]
    except Exception as e:
        print("Error parsing structured output:", e)
        return False, str(e)


# test_cpp_function = """
# void function1(const std::vector<int> &v)
# {
#     for (int i = 0; i < v.size(); ++i)
#     {
#         for (int j = 0; j < v.size(); ++j)
#         {
#             std::cout << v[i] << "," << v[j] << std::endl;
#         }
#     }
# }
# """
# success, time_complexity = analyze_time_complexity(test_cpp_function)
# print("Success:", success)
# print("Time Complexity:", time_complexity)
