# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

set(ALL_EXAMPLES )

message("Examples to be built: ${ALL_EXAMPLES}")

foreach(example ${ALL_EXAMPLES})
    add_executable(beman.constexpr_callables.examples.${example})
    target_sources(beman.constexpr_callables.examples.${example} PRIVATE ${example}.cpp)
    target_link_libraries(beman.constexpr_callables.examples.${example} beman::constexpr_callables)
endforeach()
