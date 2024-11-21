cmake_minimum_required(VERSION 3.10)

project(Flow VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------

option(flow-runner_USE_EXTERNAL_FLOW_CORE "Use an external Flow Core library" OFF)

if (flow-runner_USE_EXTERNAL_FLOW_CORE)
  find_package(nlohmann_json REQUIRED)
  find_package(flow-core REQUIRED)
endif()

add_subdirectory(third_party)

# -----------------------------------------------------------------------------
# Executable
# -----------------------------------------------------------------------------

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
endif()

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE flow-core::flow-core spdlog::spdlog cxxopts)
target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        $<$<CONFIG:Debug>:SPDLOG_ACTIVE_LEVEL=SPDLOG_TRACE>
        $<$<CONFIG:Release>:SPDLOG_ACTIVE_LEVEL=SPDLOG_INFO>
)

if(MSVC)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
  )
endif()
