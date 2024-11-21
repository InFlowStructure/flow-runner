add_subdirectory(cxxopts)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_subdirectory(spdlog)

if (NOT flow-runner_USE_EXTERNAL_FLOW_CORE)
    add_subdirectory(flow-core)
endif()