# flow-runner

[![CMake](https://github.com/InFlowStructure/flow-runner/actions/workflows/cmake.yml/badge.svg)](https://github.com/InFlowStructure/flow-runner/actions/workflows/cmake.yml)

A simple program for running flows.

## Building

To build the project with CMake, simply run
```bash
cmake -B build
cmake --build build --parallel
```

## Running

To run a basic flow, run the following
```cmake
Flow -f <path/to/file.flow>
```