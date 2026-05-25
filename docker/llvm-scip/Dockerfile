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
      python3.10-full python3.10-dev python3.11-full python3.11-dev python3.12-full python3.12-dev \
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

ENV MLIR_DIR=/toolchain/llvm

# Let us use `source` in RUN steps
SHELL ["/bin/bash", "-o", "pipefail", "-c"]

# Create venvs (3.10, 3.11 and 3.12) and install MLIR Python binding requirements (from the submodule)
ENV VENV_DIR_310=/toolchain/llvm-venv-310
RUN /usr/bin/python3.10 -m venv "${VENV_DIR_310}" \
 && source "${VENV_DIR_310}/bin/activate" \
 && python -m pip install --upgrade pip build wheel setuptools \
 && pip install -r ${MLIR_DIR}/mlir/python/requirements.txt \

ENV VENV_DIR_311=/toolchain/llvm-venv-311
RUN /usr/bin/python3.11 -m venv "${VENV_DIR_311}" \
 && source "${VENV_DIR_311}/bin/activate" \
 && python -m pip install --upgrade pip build wheel setuptools \
 && pip install -r ${MLIR_DIR}/mlir/python/requirements.txt \

ENV VENV_DIR_312=/toolchain/llvm-venv-312
RUN /usr/bin/python3.12 -m venv "${VENV_DIR_312}" \
 && source "${VENV_DIR_312}/bin/activate" \
 && python -m pip install --upgrade pip build wheel setuptools \
 && pip install -r ${MLIR_DIR}/mlir/python/requirements.txt

# Build & install LLVM/MLIR
# We use the python 3.10 venv for building LLVM. This is *not* an issue for
# compiling wheels for python 3.11 and python 3.12 later, since "most" of the
# code generated for python 3.10 will run on python 3.11 and 3.12.
RUN mkdir -p ${MLIR_DIR}/build && cd ${MLIR_DIR}/build \
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
      -DPython3_EXECUTABLE="${VENV_DIR_310}/bin/python3" \
 && cmake --build .

ENV PATH=${MLIR_DIR}/build/bin:${PATH}
ENV MLIR_CMAKE_DIR=${MLIR_DIR}/build/lib/cmake/mlir

ARG SCIP_VERSION=9.2.2

RUN wget -q https://www.scipopt.org/download/release/SCIPOptSuite-${SCIP_VERSION}-Linux-ubuntu24.deb \
    && apt -y install ./SCIPOptSuite-${SCIP_VERSION}-Linux-ubuntu24.deb \
    && rm SCIPOptSuite-${SCIP_VERSION}-Linux-ubuntu24.deb

ENV SCIP_DIR=/usr/lib/cmake/scip

# We leave a script that is able to return the paths of the toolchains
# We use this script in CI/CD:
# docker run --rm <image_tag> /toolchain/get_path.sh <scip|python310|python311|python312>
RUN printf "#!/bin/bash \
\n# Bash script used to easily get the locations of the LLVM toolchain and python venvs \
\ncase \$1 in \
\n\tscip) \
\n\t\tprintf '\"SCIP_DIR_IN_IMAGE=${SCIP_DIR}\"' \
\n\t\t;; \
\n\tmlir) \
\n\t\tprintf '\"MLIR_DIR_IN_IMAGE=${MLIR_CMAKE_DIR}\"' \
\n\t\t;; \
\n\tpython310) \
\n\t\tprintf '\"VENV_DIR_IN_IMAGE=${VENV_DIR_310}\"' \
\n\t\t;; \
\n\tpython311) \
\n\t\tprintf '\"VENV_DIR_IN_IMAGE=${VENV_DIR_311}\"' \
\n\t\t;; \
\n\tpython312) \
\n\t\tprintf '\"VENV_DIR_IN_IMAGE=${VENV_DIR_312}\"' \
\n\t\t;; \
\n\t*) \
\n\t\tprintf 'Unknown' \
\n\t\t;; \
\nesac" > get_path.sh

RUN chmod +x script.sh

# We set this one as the workdir for builds
WORKDIR /workspace
