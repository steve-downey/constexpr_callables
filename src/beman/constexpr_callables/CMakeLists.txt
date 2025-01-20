# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

add_library(beman.constexpr_callables STATIC)
add_library(beman::constexpr_callables ALIAS beman.constexpr_callables)

target_sources(
    beman.constexpr_callables
    PRIVATE function_ref.cpp copyable_function.cpp move_only_function.cpp
)

target_sources(
    beman.constexpr_callables
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${PROJECT_SOURCE_DIR}/include
        FILES
            ${PROJECT_SOURCE_DIR}/include/beman/constexpr_callables/function_ref.hpp
            ${PROJECT_SOURCE_DIR}/include/beman/constexpr_callables/copyable_function.hpp
            ${PROJECT_SOURCE_DIR}/include/beman/constexpr_callables/move_only_function.hpp
)

set_target_properties(
    beman.constexpr_callables
    PROPERTIES VERIFY_INTERFACE_HEADER_SETS ON
)

install(
    TARGETS beman.constexpr_callables
    EXPORT beman.constexpr_callables
    DESTINATION
    $<$<CONFIG:Debug>:debug/>${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION $<$<CONFIG:Debug>:debug/>${CMAKE_INSTALL_BINDIR}
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
