# CMakeLists.txt                                                    -*-CMake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.25)

project(
    beman.constexpr_callables # CMake Project Name, which is also the name of the top-level
    # targets (e.g., library, executable, etc.).
    DESCRIPTION "Constexpr Callables"
    LANGUAGES CXX
)

# [CMAKE.SKIP_TESTS]
option(
    BEMAN_CONSTEXPR_CALLABLES_BUILD_TESTS
    "Enable building tests and test infrastructure. Default: ON. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

# [CMAKE.SKIP_EXAMPLES]
option(
    BEMAN_CONSTEXPR_CALLABLES_BUILD_EXAMPLES
    "Enable building examples. Default: ON. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

include(FetchContent)
include(GNUInstallDirs)

if(BEMAN_CONSTEXPR_CALLABLES_BUILD_TESTS)
    enable_testing()

    # Fetch GoogleTest
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG
            b514bdc898e2951020cbdca1304b75f5950d1f59 # release-1.15.2
        EXCLUDE_FROM_ALL
    )
    block()
        set(INSTALL_GTEST OFF) # Disable GoogleTest installation
        FetchContent_MakeAvailable(googletest)
    endblock()
endif()

add_subdirectory(src/beman/constexpr_callables)

if(BEMAN_CONSTEXPR_CALLABLES_BUILD_TESTS)
    add_subdirectory(tests/beman/constexpr_callables)
endif()

if(BEMAN_CONSTEXPR_CALLABLES_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
