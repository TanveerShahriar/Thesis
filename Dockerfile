# Base Image: Use Ubuntu 22.04 for compatibility
FROM ubuntu:22.04

# Install required dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    python3 \
    python3-pip \
    libffi-dev \
    zlib1g-dev \
    libssl-dev \
    libblas-dev \
    liblapack-dev \
    gfortran \
    libxml2-dev \
    libxrender1 \
    libxext6 \
    libsm6 \
    git \
    wget \
    valgrind \
    libpfm4-dev \
    libunwind-dev \
    libjemalloc-dev \
    && rm -rf /var/lib/apt/lists/*

# Set environment variables
ENV LLVM_SRC=/llvm-project \
    LLVM_BUILD=/llvm-project/build \
    LLVM_INSTALL=/usr/local/llvm-latest

# Clone the latest LLVM/Clang source code
RUN git clone https://github.com/llvm/llvm-project.git $LLVM_SRC

# Create a build directory and configure the build
RUN mkdir -p $LLVM_BUILD && cd $LLVM_BUILD && \
    cmake -G Ninja \
    -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" \
    -DLLVM_TARGETS_TO_BUILD="X86" \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_ENABLE_RTTI=ON \
    -DLLVM_ENABLE_EH=ON \
    -DCLANG_BUILD_TOOLS=ON \
    -DCMAKE_INSTALL_PREFIX=$LLVM_INSTALL \
    ../llvm

# Build and install LLVM/Clang
RUN cd $LLVM_BUILD && ninja -j 8 && ninja install

# Add the installed Clang to PATH
ENV PATH=$LLVM_INSTALL/bin:$PATH

# Upgrade pip and install Python modules
RUN pip3 install --upgrade pip && pip3 install numpy networkx scikit-learn matplotlib markov_clustering scipy cdlib

# Set default working directory
WORKDIR /workspace

# Default command
CMD ["bash"]