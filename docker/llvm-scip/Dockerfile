FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# System deps + deadsnakes for Python 3.10
RUN apt-get update && apt-get install -y --no-install-recommends \
    software-properties-common \
    ca-certificates curl wget git gnupg lsb-release ccache \
    zlib1g-dev libreadline-dev libncurses-dev \
 && add-apt-repository ppa:deadsnakes/ppa -y \
 && apt-get update \
 && apt-get install -y --no-install-recommends \
      python3.10-full python3.10-dev \
 && apt-get install -y build-essential cmake ninja-build \
 && rm -rf /var/lib/apt/lists/*

# LLVM/Clang 21 + lld-21 from apt.llvm.org
RUN wget https://apt.llvm.org/llvm.sh && chmod +x llvm.sh
RUN ./llvm.sh 21 all && rm -f llvm.sh

# We need to leave the LLVM toolchain in another directory. GitHub recommends against using
# the WORKDIR directive, since it will mount GITHUB_WORKSPACE at the location of WORKDIR
# https://docs.github.com/en/actions/reference/workflows-and-actions/dockerfile-support#workdir
# W ecan avoid that by using another workir, and later setting a new one
WORKDIR /toolchain

ARG LLVM_SHA  
RUN git clone --depth 1 https://github.com/llvm/llvm-project.git llvm \
 && cd llvm && git fetch --depth 1 origin "$LLVM_SHA" && git checkout "$LLVM_SHA"

# Let us use `source` in RUN steps
SHELL ["/bin/bash", "-o", "pipefail", "-c"]

# Create venv and install MLIR Python binding requirements (from the submodule)
ENV VENV_DIR=/toolchain/llvm-venv
RUN /usr/bin/python3.10 -m venv "${VENV_DIR}" \
 && source "${VENV_DIR}/bin/activate" \
 && python -m pip install --upgrade pip wheel setuptools \
 && pip install -r /toolchain/llvm/mlir/python/requirements.txt

# Build & install LLVM/MLIR
RUN mkdir -p /toolchain/llvm/build && cd /toolchain/llvm/build \
 && cmake -G Ninja ../llvm \
      -DCMAKE_C_COMPILER=clang-21 \
      -DCMAKE_CXX_COMPILER=clang++-21 \
      -DCMAKE_LINKER=ld.lld-21 \
      -DCMAKE_INSTALL_PREFIX=/opt/mlir \
      -DLLVM_ENABLE_PROJECTS=mlir \
      -DLLVM_BUILD_EXAMPLES=ON \
      -DLLVM_TARGETS_TO_BUILD="X86" \
      -DCMAKE_BUILD_TYPE=Release \
      -DLLVM_ENABLE_ASSERTIONS=ON \
      -DLLVM_INSTALL_UTILS=ON \
      -DMLIR_ENABLE_BINDINGS_PYTHON=ON \
      -DPython3_EXECUTABLE="${VENV_DIR}/bin/python3" \
 && cmake --build .

ENV PATH=/toolchain/llvm/build/bin:${PATH}

ARG SCIP_VERSION=9.2.2

RUN wget -q https://www.scipopt.org/download/release/SCIPOptSuite-${SCIP_VERSION}-Linux-ubuntu24.deb \
    && apt -y install ./SCIPOptSuite-${SCIP_VERSION}-Linux-ubuntu24.deb \
    && rm SCIPOptSuite-${SCIP_VERSION}-Linux-ubuntu24.deb

ENV SCIP_DIR=/usr/lib/cmake/scip

# We set this one as the workdir for builds
WORKDIR /workspace
