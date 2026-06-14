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

# Boost headers (from transitive dependency or system)
set(BOOST_ROOT "$ENV{BOOST_ROOT}" CACHE PATH "Boost installation root")

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
if(BOOST_ROOT)
    message(STATUS "Boost root:    ${BOOST_ROOT}")
endif()

# ============================================================
# WrapIt-generated code location
# ============================================================

set(GENERATED_DIR "${CMAKE_SOURCE_DIR}/libRDKit/src")

if(NOT EXISTS "${GENERATED_DIR}/jlRDKit.cxx")
    message(FATAL_ERROR
        "WrapIt generated code not found at ${GENERATED_DIR}/jlRDKit.cxx\n"
        "Run: cd jlRDKit && julia --project=../jlRDKit_test gen/build.jl"
    )
endif()

# ============================================================
# Per-file compile definition: rename define_julia_module in WrapIt output
# so our wrapit_integration.cpp can wrap it and add shim functions
# ============================================================

set_source_files_properties("${GENERATED_DIR}/jlRDKit.cxx" PROPERTIES
    COMPILE_DEFINITIONS "define_julia_module=wrapit_module"
)

# ============================================================
# Wrapper shared library
# ============================================================

add_library(jlRDKit SHARED
    "${GENERATED_DIR}/jlRDKit.cxx"
    src/wrapit_integration.cpp
    src/rdkit_shims.cpp
)

target_include_directories(jlRDKit PRIVATE
    ${RDKIT_INCLUDE_DIR}
    ${RDKIT_INCLUDE_DIR}/..        # for #include "rdkit/..." from jlRDKit.h
    ${JLCXX_INCLUDE_DIR}
    ${JULIA_INCLUDE_DIR}
    ${GENERATED_DIR}               # Wrapper.h, dbg_msg.h, jlRDKit.h
    ${CMAKE_SOURCE_DIR}/src        # rdkit_shims.h
)

if(BOOST_ROOT)
    target_include_directories(jlRDKit PRIVATE
        "${BOOST_ROOT}/include"
        "${BOOST_ROOT}"
    )
endif()

# Link RDKit C++ libraries + JlCxx
target_link_libraries(jlRDKit PRIVATE
    ${JLCXX_LIBRARY}
    ${RDKIT_LIB_DIR}/libRDKitGraphMol.so
    ${RDKIT_LIB_DIR}/libRDKitSmilesParse.so
    ${RDKIT_LIB_DIR}/libRDKitRDGeneral.so
    ${RDKIT_LIB_DIR}/libRDKitRDGeometryLib.so
    ${RDKIT_LIB_DIR}/libRDKitSubstructMatch.so
    ${RDKIT_LIB_DIR}/libRDKitFileParsers.so
)

# Output library to build/lib/ — loaded by RDKit.jl via JLRDKIT_LIB_PATH
set_target_properties(jlRDKit PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    BUILD_RPATH "${JLCXX_ROOT}/lib;${RDKIT_LIB_DIR};${BOOST_ROOT}/lib"
    INSTALL_RPATH "${JLCXX_ROOT}/lib;${RDKIT_LIB_DIR};${BOOST_ROOT}/lib"
)
