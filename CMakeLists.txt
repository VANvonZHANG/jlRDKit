cmake_minimum_required(VERSION 3.15)
project(jlRDKit LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# 依赖路径（来自 RDKit_jll + libcxxwrap_julia_jll artifacts）
# ============================================================

# RDKit_jll artifact
set(RDKIT_ROOT "/data/zhangfan_data/julia_env/artifacts/f75c34cbe6e791127e1e6381ec91fa3fd9025abd")
set(RDKIT_INCLUDE_DIR "${RDKIT_ROOT}/include/rdkit")
set(RDKIT_LIB_DIR "${RDKIT_ROOT}/lib")

# JlCxx (libcxxwrap_julia_jll artifact)
set(JLCXX_ROOT "/data/zhangfan_data/julia_env/artifacts/c74f464ba08d47b17764bb8c69c43b8213993c45")
set(JLCXX_INCLUDE_DIR "${JLCXX_ROOT}/include")
set(JLCXX_LIBRARY "${JLCXX_ROOT}/lib/libcxxwrap_julia.so")

# Julia headers
set(JULIA_INCLUDE_DIR "/home/zhangfan/julia-1.10.10/include/julia")

# 验证关键文件存在
foreach(_path
    "${RDKIT_INCLUDE_DIR}/GraphMol/ROMol.h"
    "${RDKIT_INCLUDE_DIR}/GraphMol/Atom.h"
    "${RDKIT_LIB_DIR}/libRDKitGraphMol.so"
    "${JLCXX_INCLUDE_DIR}/jlcxx/jlcxx.hpp"
    "${JLCXX_LIBRARY}"
    "${JULIA_INCLUDE_DIR}/julia.h"
)
    if(NOT EXISTS "${_path}")
        message(FATAL_ERROR "Required file not found: ${_path}")
    endif()
endforeach()

message(STATUS "RDKit include: ${RDKIT_INCLUDE_DIR}")
message(STATUS "RDKit lib:     ${RDKIT_LIB_DIR}")
message(STATUS "JlCxx include: ${JLCXX_INCLUDE_DIR}")
message(STATUS "JlCxx library: ${JLCXX_LIBRARY}")

# ============================================================
# 编译 wrapper 共享库
# ============================================================

add_library(RDKitWrap SHARED src/rdkit_wrapper.cpp)

target_include_directories(RDKitWrap PRIVATE
    ${RDKIT_INCLUDE_DIR}
    ${JLCXX_INCLUDE_DIR}
    ${JULIA_INCLUDE_DIR}
)

# 链接 RDKit C++ 库 + JlCxx
target_link_libraries(RDKitWrap PRIVATE
    ${JLCXX_LIBRARY}
    ${RDKIT_LIB_DIR}/libRDKitGraphMol.so
    ${RDKIT_LIB_DIR}/libRDKitSmilesParse.so
    ${RDKIT_LIB_DIR}/libRDKitRDGeneral.so
    ${RDKIT_LIB_DIR}/libRDKitRDGeometryLib.so
)

# 设置 RPATH，确保运行时能找到 JlCxx 和 RDKit 库
set_target_properties(RDKitWrap PROPERTIES
    BUILD_RPATH "${JLCXX_ROOT}/lib;${RDKIT_LIB_DIR}"
    INSTALL_RPATH "${JLCXX_ROOT}/lib;${RDKIT_LIB_DIR}"
)
