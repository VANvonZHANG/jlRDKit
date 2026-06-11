#!/usr/bin/env julia
# gen/build.jl — Generate CxxWrap wrapper code via WrapIt, then build with CMake
# Usage: julia --project=../jlRDKit_test gen/build.jl
#
# This script resolves all dependency paths dynamically from Julia JLL packages,
# generates config.toml from config.toml.in, runs WrapIt, and invokes CMake.

using CxxWrap
using WrapIt
using RDKit_jll

# ============================================================
# 1. Resolve dependency paths dynamically
# ============================================================

rdkit_prefix   = RDKit_jll.artifact_dir
cxxwrap_prefix = CxxWrap.prefix_path()
julia_prefix   = dirname(Sys.BINDIR)

# Boost is a transitive dependency of RDKit_jll and not directly loadable.
# Find it by scanning the Julia artifacts directory for boost headers.
function find_boost_prefix(artifacts_dir::String)
    for d in readdir(artifacts_dir)
        candidate = joinpath(artifacts_dir, d, "include", "boost")
        if isdir(candidate) && isfile(joinpath(candidate, "version.hpp"))
            return joinpath(artifacts_dir, d)
        end
    end
    error("Could not find boost artifact in $(artifacts_dir)")
end

boost_prefix = find_boost_prefix(dirname(rdkit_prefix))

println("Resolved dependency paths:")
println("  RDKit prefix:   ", rdkit_prefix)
println("  JlCxx prefix:   ", cxxwrap_prefix)
println("  Julia prefix:   ", julia_prefix)
println("  Boost prefix:   ", boost_prefix)

# ============================================================
# 2. Generate config.toml from template
# ============================================================

template = joinpath(@__DIR__, "config.toml.in")
config   = joinpath(@__DIR__, "config.toml")

println("\nGenerating config.toml from template...")
open(config, "w") do f
    for l in eachline(template)
        l = replace(l, "@RDKIT_INCLUDE_DIR@"      => "$(rdkit_prefix)/include/rdkit")
        l = replace(l, "@RDKIT_ROOT_INCLUDE_DIR@" => "$(rdkit_prefix)/include")
        l = replace(l, "@JLCXX_INCLUDE_DIR@"      => "$(cxxwrap_prefix)/include")
        l = replace(l, "@JULIA_INCLUDE_DIR@"       => "$(julia_prefix)/include/julia")
        l = replace(l, "@BOOST_INCLUDE_DIR@"       => "$(boost_prefix)/include/boost")
        l = replace(l, "@BOOST_ROOT_INCLUDE_DIR@"  => "$(boost_prefix)/include")
        println(f, l)
    end
end
println("Config written to: ", config)

# ============================================================
# 3. Run WrapIt to generate CxxWrap wrapper code
# ============================================================

println("\nRunning WrapIt...")
rc = wrapit(config; force=true, verbosity=1, returncode=true)
if rc !== nothing && rc != 0
    error("WrapIt failed with exit code: ", rc)
end
println("WrapIt done! Generated code in libRDKit/")

# ============================================================
# 3b. Post-process generated code to fix known WrapIt issues
# ============================================================

sourcedir = dirname(@__DIR__)
generated_cxx = joinpath(sourcedir, "libRDKit", "src", "jlRDKit.cxx")

if isfile(generated_cxx)
    println("\nPost-processing generated code...")
    content = read(generated_cxx, String)
    original_len = length(content)

    # RDProps is not polymorphic (no virtual destructor), so WrapIt-generated
    # SuperType<T>::type = RDProps and julia_base_type<RDProps>() calls cause
    # compilation errors (cannot dynamic_cast from non-polymorphic type).
    # Remove all SuperType and julia_base_type lines that reference RDProps.
    content = replace(content, r"^\s*template<>\s*struct\s*SuperType<[^>]+>\s*\{\s*typedef\s*RDKit::RDProps\s*type;\s*\};\s*$"m => "")

    # Remove the julia_base_type<RDProps>() calls (appear in set_base calls)
    content = replace(content, r",\s*jlcxx::julia_base_type<RDKit::RDProps>\(\)" => "")

    # If any set_base calls now have empty trailing parens, clean them up
    # (this shouldn't happen given the replacement above, but just in case)

    if length(content) != original_len
        write(generated_cxx, content)
        println("  Patched RDProps inheritance issues")
    else
        println("  No RDProps patches needed")
    end
else
    @warn "Generated file not found: $generated_cxx"
end

# ============================================================
# 4. Build with CMake
# ============================================================

builddir  = joinpath(sourcedir, "build")
mkpath(builddir)

cmake_prefix_path = "$(cxxwrap_prefix);$(rdkit_prefix);$(boost_prefix)"

println("\nRunning CMake...")
cd(builddir)
run(`cmake -S $(sourcedir) -B $(builddir)
     -DCMAKE_BUILD_TYPE=Release
     -DRDKIT_ROOT=$(rdkit_prefix)
     -DJLCXX_ROOT=$(cxxwrap_prefix)
     -DJULIA_ROOT=$(julia_prefix)
     -DBOOST_ROOT=$(boost_prefix)
     -DCMAKE_PREFIX_PATH=$(cmake_prefix_path)`)

println("\nBuilding...")
run(`cmake --build $(builddir) --config Release --parallel $(Sys.CPU_THREADS)`)

println("\nBuild complete!")
