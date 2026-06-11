#!/usr/bin/env julia
# gen/build.jl — 运行 WrapIt 生成 CxxWrap 包装代码
# 用法: julia --project=../jlRDKit_test gen/build.jl

using WrapIt

config = joinpath(@__DIR__, "config.toml")
println("Running WrapIt with config: ", config)
wrapit(config; force=true, verbosity=1)
println("Done! Generated code in libRDKit/")
