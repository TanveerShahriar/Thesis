# Time Complexity Analyzer

## Overview

The **Time Complexity Analyzer** is a Python-based tool designed to analyze C++ functions and estimate their time complexity. It uses AI models to generate valid C++ expressions representing the estimated number of basic operations required to execute a given function. The tool also provides a fallback mechanism to count the total number of statements in the function if the AI fails to determine the time complexity.

---

## Features

- Extracts functions from C++ source files.
- Analyzes the time complexity of each function using an AI model.
- Generates a C++ header file (`cpp_functions.h`) containing a hashmap of function names and their corresponding time complexities.

---

## Requirements

### Python Version

-**Python 3.10 or below** is required to run this project. Ensure your Python version is compatible by running:

```bash
  python --version
```


### AI Model
* Download the AI model file: [**`DeepSeek-Coder-V2-Lite-Instruct-Q4_K_M.gguf`**](https://drive.google.com/file/d/1B6e-l6FLLNyFQzNW9k0_i2j9Y1yZvxZK/view?usp=sharing).
* Place the model file in the Model directory as the Python scripts.

### Dependencies

Install the required Python libraries by running:

**pip** **install** **-r** **requirements.txt**

---

## Usage Instructions

### 1. Prepare the Source Code

* Place your C++ source code files in a folder named **`Input`** in the root directory of the project.
* Supported file extensions: `.cpp`, `.cc`, `.cxx`, `.c`, `.h`.

### 2. Run the Analyzer

* Execute the `main.py` script to analyze the functions:

  **python** **main.py**

### 3. Output

* The analyzed results will be saved in a C++ header file named  **`cpp_functions.h`** .
* The output file will be generated in the **`Obfuscator`** folder in the root directory.

---

## Notes

* If the AI model fails to determine the time complexity, the tool will use the total number of statements in the function as a fallback.
* The tool attempts to analyze each function up to 5 times before falling back to the statement count.

---