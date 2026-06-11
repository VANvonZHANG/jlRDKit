cmake_minimum_required(VERSION 3.15)
project(jlRDKit LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# Dependency paths (passed from gen/build.jl or set via environment)
# ============================================================

# RDKit (from RDKit_jll)
set(RDKIT_ROOT "$ENV{RDKIT_ROOT}" CACHE PATH "RDKit installation root")
set(RDKIT_INCLUDE_DIR "${RDKIT_ROOT}/include/rdkit")
set(RDKIT_LIB_DIR "${RDKIT_ROOT}/lib")

# JlCxx (from CxxWrap.prefix_path())
set(JLCXX_ROOT "$ENV{JLCXX_ROOT}" CACHE PATH "JlCxx installation root")
set(JLCXX_INCLUDE_DIR "${JLCXX_ROOT}/include")
set(JLCXX_LIBRARY "${JLCXX_ROOT}/lib/libcxxwrap_julia.so")

# Julia headers
set(JULIA_ROOT "$ENV{JULIA_ROOT}" CACHE PATH "Julia installation root")
set(JULIA_INCLUDE_DIR "${JULIA_ROOT}/include/julia")

# Verify required files exist
foreach(_path
    "${RDKIT_INCLUDE_DIR}/GraphMol/ROMol.h"
    "${RDKIT_INCLUDE_DIR}/GraphMol/Atom.h"
    "${RDKIT_LIB_DIR}/libRDKitGraphMol.so"
    "${JLCXX_INCLUDE_DIR}/jlcxx/jlcxx.hpp"
    "${JLCXX_LIBRARY}"
    "${JULIA_INCLUDE_DIR}/julia.h"
)
    if(NOT EXISTS "${_path}")
        message(FATAL_ERROR "Required file not found: ${_path}\n"
            "Ensure RDKIT_ROOT, JLCXX_ROOT, JULIA_ROOT are set correctly.\n"
            "Run: julia --project=../jlRDKit_test gen/build.jl")
    endif()
endforeach()

message(STATUS "RDKit include: ${RDKIT_INCLUDE_DIR}")
message(STATUS "RDKit lib:     ${RDKIT_LIB_DIR}")
message(STATUS "JlCxx include: ${JLCXX_INCLUDE_DIR}")
message(STATUS "JlCxx library: ${JLCXX_LIBRARY}")
message(STATUS "Julia include: ${JULIA_INCLUDE_DIR}")

# ============================================================
# Wrapper shared library
# ============================================================

add_library(RDKitWrap SHARED
    src/rdkit_module.cpp
    src/rdkit_shims.cpp
)

target_include_directories(RDKitWrap PRIVATE
    ${RDKIT_INCLUDE_DIR}
    ${JLCXX_INCLUDE_DIR}
    ${JULIA_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/src    # rdkit_shims.h
)

# Link RDKit C++ libraries + JlCxx
target_link_libraries(RDKitWrap PRIVATE
    ${JLCXX_LIBRARY}
    ${RDKIT_LIB_DIR}/libRDKitGraphMol.so
    ${RDKIT_LIB_DIR}/libRDKitSmilesParse.so
    ${RDKIT_LIB_DIR}/libRDKitRDGeneral.so
    ${RDKIT_LIB_DIR}/libRDKitRDGeometryLib.so
    ${RDKIT_LIB_DIR}/libRDKitSubstructMatch.so
)

# RPATH for runtime library discovery
set_target_properties(RDKitWrap PROPERTIES
    BUILD_RPATH "${JLCXX_ROOT}/lib;${RDKIT_LIB_DIR}"
    INSTALL_RPATH "${JLCXX_ROOT}/lib;${RDKIT_LIB_DIR}"
)
