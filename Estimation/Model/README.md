# Required Model File: DeepSeek-Coder-V2-Lite-Instruct-Q4_K_M.gguf

## Important Notice

For the Time Complexity Analyzer to function properly, you must place the required model file in the correct location:

```
/Estimation/Model/DeepSeek-Coder-V2-Lite-Instruct-Q4_K_M.gguf
```

## About the Model

The Time Complexity Analyzer uses the **DeepSeek Coder V2 Lite** model (specifically the quantized Q4_K_M version) to analyze C++ functions and determine their time complexity. This is a large language model specialized for code understanding and generation tasks.

## Download Information

The model file `DeepSeek-Coder-V2-Lite-Instruct-Q4_K_M.gguf` is a quantized version of the DeepSeek Coder model, optimized for efficiency while maintaining good performance. You can download this model file from:

-   **Google Drive**: [DeepSeek-Coder-V2-Lite-Instruct-Q4_K_M.gguf](https://drive.google.com/file/d/1B6e-l6FLLNyFQzNW9k0_i2j9Y1yZvxZK/view?usp=sharing)
-   **HuggingFace**: [DeepSeek-Coder-V2-Lite-Instruct-GGUF](https://huggingface.co/deepseek-ai/deepseek-coder-v2-lite-instruct/tree/main)

## File Size Warning

Please note that this model file is large (approximately 3.5GB). Ensure you have sufficient disk space and a stable internet connection when downloading.

## Using a Different Model

If you want to use a different model:

1. Download your preferred model file (must be compatible with LlamaCpp)
2. Place it in the `Estimation/Model/` directory
3. Open the `time_complexity_analyzer.py` file and modify line 6:

```python
model_path = "./Model/DeepSeek-Coder-V2-Lite-Instruct-Q4_K_M.gguf"
```

Replace it with the path to your model file:

```python
model_path = "./Model/YOUR_MODEL_FILENAME.gguf"
```

Additionally, you may need to adjust other parameters in the LlamaCpp initialization (lines 8-13) depending on your model's requirements:

```python
llm = LlamaCpp(
    model_path=model_path,
    n_ctx=13107,         # Context window size
    n_gpu_layers=20,     # Number of layers to offload to GPU
    verbose=False,
    temperature=0.0      # Deterministic output
)
```

## Model Requirements

For optimal performance:

-   The model should be capable of understanding and analyzing C++ code
-   It should be compatible with the LlamaCpp library
-   Quantized models (like Q4_K_M) provide a good balance between performance and resource usage

## Troubleshooting

If you encounter errors related to the model file:

1. Verify that the file exists in the specified path
2. Check that the filename matches exactly what's specified in `time_complexity_analyzer.py`
3. Ensure the model is compatible with LlamaCpp
4. Check if your system has enough resources (RAM/GPU memory) to load the model

## Note on Hardware Requirements

Running this model requires:

-   Minimum 8GB RAM (16GB recommended)
-   GPU with at least 4GB VRAM for optimal performance (using GPU acceleration)
-   Approximately 4GB of free disk space for the model file

Without the proper model file in place, the analyzer will fail to initialize and you'll encounter errors when running the application.
