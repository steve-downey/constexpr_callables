# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

if(BEMAN_CONSTEXPR_CALLABLES_USE_MODULES)
    target_sources(
        beman.constexpr_callables
        PUBLIC
            FILE_SET CXX_MODULES FILES constexpr_callables.cppm
            FILE_SET HEADERS
                FILES
                    config.hpp
                    constexpr_callables.hpp
                    copyable_function.hpp
                    detail/call_base.hpp
                    detail/call_base_impl.hpp
                    detail/function_ref_impl.hpp
                    function_ref.hpp
                    move_only_function.hpp
                    "${PROJECT_BINARY_DIR}/include/beman/constexpr_callables/config_generated.hpp"
    )
else()
    target_sources(
        beman.constexpr_callables
        PUBLIC
            FILE_SET HEADERS
                FILES
                    config.hpp
                    constexpr_callables.hpp
                    copyable_function.hpp
                    detail/call_base.hpp
                    detail/call_base_impl.hpp
                    detail/function_ref_impl.hpp
                    function_ref.hpp
                    move_only_function.hpp
                    "${PROJECT_BINARY_DIR}/include/beman/constexpr_callables/config_generated.hpp"
    )
endif()
