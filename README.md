# Qoala MLIR

## LLVM git submodule
*IMPORTANT:* This repository contains a link to the LLVM repository as a git submodule.
Right after checking out this repository, you need to initialize and checkout the LLVM
submodule by running:
```sh
git submodule update --init
```

## Required packages
Before going into the compilation, we need to install a few build tools. In Debian-like distros:
```shell
sudo apt-get install build-essential cmake ninja-build python3-full python3-dev
```

## Recommended packages
By default, you can use `gcc` as the compiler for LLVM and this repo, but `clang` works much faster
and it's better for optimization.

To install clang, you can follow the [official LLVM documentation (for Debian-based distros)](https://apt.llvm.org/).
To install clang/LLVM 17 run:
```shell
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 17 all
```

## Prepare the python environment
Create a python virtual environment:
```shell
cd /path/to/your/venvs
python3 -m venv llvm-venv # "llvm-venv" is the name of out python virtual environment
```

Activate the python virtual environment and install the [requirements for the MLIR python bindings](https://mlir.llvm.org/docs/Bindings/Python/#pre-requisites)
```shell
source /path/to/your/venvs/llvm-venv/bin/activate
(llvm-venv)$ cd llvm # The root folder of the llvm project
(llvm-venv)$ pip install -r mlir/python/requirements.txt
```

## Build LLVM
_With the python virtual environment activated_, navigate to the `./llvm` directory (top level of submodule):

Create `build` directory:
```shell
mkdir build && cd build
```

Configure (Optionally, set the clang compiler here. Please change the suffix `-17` to match the LLVM version you have).
**IMPORTANT**: It is highly recommended to use the `-DCMAKE_INSTALL_PREFIX` option to configure a different
installation prefix of the LLVM/MLIR files. In the example below, we use the prefix `/opt/mlir`, so LLVM/MLIR will be
installed in the `/opt/mlir/` folder. This is needed to _avoid leaving another clang/LLVM installation in an
inconsistent state_, so we can use clang as the compiler, and the required version of the MLIR for this repository.
Please also note that the installation prefix needs to be configured **before compiling** LLVM/MLIR; it **cannot**
be specified when running the `install` target later.
```shell
cmake -G Ninja ../llvm \
      -DCMAKE_C_COMPILER=clang-17 \
      -DCMAKE_CXX_COMPILER=clang++-17 \
      -DCMAKE_LINKER=ld.lld-17 \
      -DCMAKE_INSTALL_PREFIX=/opt/mlir \
      -DLLVM_ENABLE_PROJECTS=mlir \
      -DLLVM_BUILD_EXAMPLES=ON \
      -DLLVM_TARGETS_TO_BUILD="X86" \
      -DCMAKE_BUILD_TYPE=Release \
      -DLLVM_ENABLE_ASSERTIONS=ON \
      -DLLVM_INSTALL_UTILS=ON \
      -DMLIR_ENABLE_BINDINGS_PYTHON=ON \
      -DPython3_EXECUTABLE=/path/to/your/venvs/llvm-venv/bin/python3
```
*Warning*: We need to specify the python executable _from the virtual environment_. Otherwise, the MLIR python
requirements will not be available when needed.

Build llvm according to the configuration (should produce the compiled files in `./llvm/build`)
```shell
cmake --build .
```

Install (should install all the binaries in `lib`, `include` and `bin` folders under the folder given with the
`CMAKE_INSTALL_PREFIX` options. Please note that this base install folder _cannot be changed_ without needing to
recompile the whole llvm project):
```shell
sudo cmake --build . --target install
```

## Build this repo

This setup assumes that you have built LLVM and MLIR in `./llvm/build`, MLIR has been installed to `/opt/mlir`
(as configured in the LLVM/MLIR compilation line shown above), and your python virtual environment is located in the
folder `/path/to/your/venvs/llvm-venv`. Before running this command, please make sure that you have activated the python
virtual environment created for LLVM.
To build everything, run (see below to use `clang/LLVM` as the compiler toolchain)
```shell
(llvm-venv)$ mkdir build && cd build
(llvm-venv)$ cmake -G Ninja .. -DMLIR_DIR=/opt/mlir/lib/cmake/mlir -DPython3_EXECUTABLE=/path/to/your/venvs/llvm-venv/bin/python3
(llvm-venv)$ cmake --build . 
```

To compile with `clang`, execute these commands (Please change the suffix `-17` to match the LLVM version you have):
```shell
(llvm-venv)$ mkdir build && cd build
(llvm-venv)$ 
(llvm-venv)$ LD=ld.ldd-17 cmake -G Ninja .. \
                                -DCMAKE_C_COMPILER=clang-17 \
                                -DCMAKE_CXX_COMPILER=clang++-17 \
                                -DCMAKE_LINKER=ld.ldd-17 \
                                -DMLIR_DIR=/opt/mlir/lib/cmake/mlir \
                                -DPython3_EXECUTABLE=/path/to/your/venvs/llvm-venv/bin/python3
(llvm-venv)$ cmake --build . 
```

## Build a specific dialect
All from within `build/`:

Try to run tablegen for the Qnet dialect (uses stuff in `include/Dialect/Qnet`):
```shell
cmake --build . --target MLIRQnetIncGen
```

Try to build Netqasm dialect library (uses stuff in `lib/Dialect/Qnet`, relies on `MLIRQnetIncGen`):
```shell
cmake --build . --target MLIRQnet
```

Try to build Netqasm dialect optimizer tool (uses stuff in `tools/qnet`, relies on `MLIRQnet`):
```shell
cmake --build . --target qoala-opt
```

## Run tests
Note: `qoala-opt` needs to be on your PATH.
(TODO: figure out how to point `llvm-lit` to `build/bin/qoala-opt` instead.)

```shell
./llvm/build/bin/llvm-lit test
```


## Print graph

From within the `graphs` directory:

```
../build/bin/qoala-opt ../programs/bqc_server.mlir --view-op-graph 2>&1 >/dev/null | tee bqc_server_qnet.gv
``` 

View graph with `xdot bqc_server_qnet.gv`.

## Using the generating python bindings

### Runtime requirements

Before using the python bindings, it is required to install a few python libraries in your python virtual environment.
The LLVM source project comes with a `requirements.txt` file that contains the list of packages that need to be
installed before using any MLIR python bindings.

To install the requirements, execute these commands:
```shell
source /path/to/your/venvs/runtime-venv/bin/activate
(runtime-venv)$ cd /path/to/root/llvm # The root folder of the llvm project
(runtime-venv)$ pip install -r mlir/python/requirements.txt
```

After running this, the python virtual environment is ready to  be used with the MLIR python bindings.


### Directly using the compiled libraries in the `build` folder

The simplest way to use the compiled python bindings is to add the path to the `PYTHONPATH`. There are several ways to
do this:

#### Modify your `activate` script:

A simple way to do this is to update your python path in the activation script of your virtual environment. In this way
you don't need to worry about updating this variable every time you execute a python script.

Open the `/path/to/your/venvs/runtime-venv/bin/activate` file with your favorite text editor, and add the following line
at the end of that file:
```
export PYTHONPATH=$PYTHONPATH:/path/to/this/repo/build/python_packages/qnet_bindings
```

Save the file, and reactivate the virtual environment if it was active in any terminal.

#### Execute your python script with the updated python path

A more "manual" approach is to simply set the right value of the `PYTHONPATH` variable every time you want to execute a
python script with the MLIR python bindings.

To do this, simply attach the updated python path variable before invoking the python command:
```shell
(runtime-venv)$ PYTHONPATH=$PYTHONPATH:/path/to/this/repo/build/python_packages/qnet_bindings python3 python_script.py
```
Please note that you need to add the definition _every time_ you want ot run a python script with the MLIR bindings.
Additionally, you need to add this extra setting _once you have activated your python virtual environment_.


### Building a python wheel containing the QNet python bindings and `qoala-opt` and `qoala-translate` tools

To ease the installation process, this repository also contains a `pyproject.toml` file that eases the process
of creating a wheel file to distribute the python bindings and the `qoala-opt` and `qoala-translate` tools.

To create a wheel file, we need to create a python virtual environment using the python version of the same
intended target. For example, if we intend to distribute packages for python 3.11, we need to create a virtual
environment based on python 3.11:

```shell
$ python3.11 -m venv venv-311
$ source venv-311/bin/activate
(venv-311) $
```

Once created (and activated), we need to install the MLIR python requirements in the same virtual environment:
```shell
(venv-311) $ cd <this_repository>
(venv-311) $ pip install -r llvm/mlir/python/requirements.txt
```

We also need to install the building requirements in this virtual environment:

```shell
(venv-311) $ pip install -r build-requirements.txt
```

Then, *we need to adapt the `pyproject.toml` file* so the compilation of this repository uses the just-created
python environment. Open the `pyproject.toml` file and modify the value of the `DPython3_EXECUTABLE`
argument located inside the `args` list, which can be found under the `[tool.py-build-cmake.cmake]`
section:

```toml
args = [
    "-DMLIR_DIR=/opt/mlir/lib/cmake/mlir",
    "-DPython3_EXECUTABLE=/abs/path/to/venv-311/bin/python",
    "-DCMAKE_C_COMPILER=clang-20",
    "-DCMAKE_CXX_COMPILER=clang++-20",
    "-DCMAKE_LINKER=clang-20"
]
```
Also note that this arguments make use of the fact that the MLIR headers were installed with the prefix
`/opt/mlir`. In the same way, you can also specify another compiler by modifying the `DCMAKE_C_COMPILER`,
`DCMAKE_CXX_COMPILER` and `DCMAKE_LNKER` variables.

Once all this is configured, we can trigger the building by invoking the following command:

```shell
(venv-311) $ python -m build -w
```

After some time, the wheel will be created inside the `dist` folder.
