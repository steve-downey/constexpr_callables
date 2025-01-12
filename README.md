<!--
SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
-->

# beman.constexpr_callables: Constexpr Callables

<img src="https://github.com/bemanproject/beman/blob/main/images/logos/beman_logo-beman_library_under_development.png" style="width:5%; height:auto;"> ![Continuous Integration Tests](https://github.com/steve-downey/constexpr_callables/actions/workflows/ci_tests.yml/badge.svg) ![Code Format](https://github.com/steve-downey/constexpr_callables/actions/workflows/pre-commit.yml/badge.svg)

**Implements**: [`constexpr_callables` (PnnnnRr)](https://wg21.link/PnnnnRr)

**Status**: [Under development and not yet ready for production use.](https://github.com/bemanproject/beman/blob/main/docs/BEMAN_LIBRARY_MATURITY_MODEL.md#under-development-and-not-yet-ready-for-production-use)

TODO: Describe this project.

### Usage

```c++
TODO: Copy/paste example code from examples/ here.
```

## Building beman.constexpr_callables

### Dependencies
This project has no C or C++ dependencies.

Build-time dependencies:

- `cmake`
- `ninja`, `make`, or another CMake-supported build system
  - CMake defaults to "Unix Makefiles" on POSIX systems

#### How to install dependencies

<details>
<summary>Dependencies install constexpr_callables on Ubuntu 24.04  </summary>

TODO: Update this with the appropriate versions of gcc and clang.
```shell
# Install tools:
apt-get install -y cmake make ninja-build

# Toolchains:
apt-get install                             g++-14 gcc-14                             clang-18 clang++-18 clang-19 clang++-19
```

</details>

<details>
<summary>Dependencies install constexpr_callables on MAC OS $VERSION </summary>

<!-- TODO Darius: rewrite section!-->
```shell
# TODO
```

</details>

<details>
<summary>Dependencies install constexpr_callables on Windows $VERSION  </summary>
<!-- TODO Darius: rewrite section!-->

```shell
# TODO
```

</details>

### How to build beman.constexpr_callables

This project strives to be as normal and simple a CMake project as possible.
This build workflow in particular will work,
producing a static `libbeman.constexpr_callables.a` library, ready to package with its headers:

```shell
cmake --workflow --preset gcc-debug
cmake --workflow --preset gcc-release
cmake --install build/gcc-release --prefix /opt/beman.constexpr_callables
```

<details>
<summary> Build beman.constexpr_callables (verbose logs) </summary>

```shell
# Configure beman.constexpr_callables via gcc-debug workflow for development.
$ cmake --workflow --preset gcc-debug
TODO: Run the command above, and copy its output here.

# Configure beman.constexpr_callables via gcc-release workflow for direct usage.
$ cmake --workflow --preset gcc-release
TODO: Run the command above, and copy its output here.

# Run examples.
$ build/gcc-release/examples/beman.constexpr_callables.examples.constexpr_callables_direct_usage
TODO: Run the command above, and copy its output here.

```

</details>

<details>
<summary> Install beman.constexpr_callables (verbose logs) </summary>

```shell
# Install build artifacts from `build` directory into `opt/beman.constexpr_callables` path.
$ cmake --install build/gcc-release --prefix /opt/beman.constexpr_callables
TODO: Run the command above, and copy its output here.

# Check tree.
$ tree /opt/beman.constexpr_callables
TODO: Run the command above, and copy its output here.
```

</details>

<details>
<summary> Disable tests build </summary>

To build this project with tests disabled (and their dependencies),
simply use `BEMAN_CONSTEXPR_CALLABLES_BUILD_TESTING=OFF` as documented in upstream [CMake documentation](https://cmake.org/cmake/help/latest/module/CTest.html):

```shell
cmake -B build -S . -DBEMAN_CONSTEXPR_CALLABLES_BUILD_TESTING=OFF
```

</details>

## Integrate beman.constexpr_callables into your project

<details>
<summary> Use beman.constexpr_callables directly from C++ </summary>

This library is header only.  If you want to use `beman.constexpr_callables` from your
project, you can include `beman/constexpr_callables/*.hpp` files from your C++ source
files

```cpp
#include <beman/constexpr_callables/TODO.hpp>
```

and directly link with `libbeman.constexpr_callables.a`

```shell
# Assume /opt/beman.constexpr_callables staging directory.
$ c++ -o constexpr_callables_usage examples/constexpr_callables_direct_usage.cpp     -I /opt/beman.constexpr_callables/include/     -L/opt/beman.constexpr_callables/lib/ -lbeman.constexpr_callables
```

</details>

<details>
<summary> Use beman.constexpr_callables directly from CMake </summary>

<!-- TODO Darius: rewrite section! Add examples. -->

For CMake based projects, you will need to use the `beman.constexpr_callables` CMake module to define the `beman::constexpr_callables` CMake target:

```cmake
find_package(beman.constexpr_callables REQUIRED)
```

You will also need to add `beman::constexpr_callables`
to the link libraries of any libraries or executables that include `beman/constexpr_callables/*.hpp` in their source or header file.

```cmake
target_link_libraries(yourlib PUBLIC beman::constexpr_callables)
```

</details>

<details>
<summary> Use beman.constexpr_callables from other build systems </summary>

<!-- TODO Darius: rewrite section! Add examples. -->

Build systems that support `pkg-config` by providing a `beman.constexpr_callables.pc` file.
Build systems that support interoperation via `pkg-config` should be able to detect `beman.constexpr_callables` for you automatically.

</details>

### Compiler support

GCC 14 or later; Clang 18 or later; or VS 2022 or later.

Building this repository requires **C++26** or later.
