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

    # std::basic_string_view wrapper causes "No appropriate factory for type
    # std::char_traits<char>" because CxxWrap doesn't handle char_traits.
    # Remove all traces: template specializations, factory function, forward declarations.

    # 1. Remove the namespace jlcxx block with basic_string_view template specializations
    #    (BuildParameterList, IsMirroredType, DefaultConstructible) - may or may not be present.
    #    Use a dotall regex to match the entire block from "namespace jlcxx {" to the
    #    matching "}" that contains "basic_string_view".
    content = replace(content, r"namespace jlcxx \{[^}]*?BuildParameterList<std::basic_string_view[\s\S]*?\n\}"s => "")

    # 2. Remove the entire block: comment + struct definition + factory function.
    #    Matches from "// Class generating the wrapper for type std::basic_string_view"
    #    through the closing of newJlstd_basic_string_view(...){...}
    content = replace(content, r"// Class generating the wrapper for type std::basic_string_view\n// signature to use in the veto file: std::basic_string_view\nstruct Jlstd_basic_string_view[\s\S]*?^std::shared_ptr<Wrapper> newJlstd_basic_string_view\(jlcxx::Module& module\)\{\n  return std::shared_ptr<Wrapper>\(new Jlstd_basic_string_view\(module\)\);\n\}"m => "")

    # 3. Remove forward class declaration: "class Jlstd_basic_string_view;"
    content = replace(content, r"^class Jlstd_basic_string_view;\n"m => "")

    # 4. Remove forward function declaration: "std::shared_ptr<Wrapper> newJlstd_basic_string_view(jlcxx::Module&);"
    content = replace(content, r"^std::shared_ptr<Wrapper> newJlstd_basic_string_view\(jlcxx::Module&\);\n"m => "")

    # 5. Remove factory line from wrappers vector (if present)
    content = replace(content, r"^\s*std::shared_ptr<Wrapper>\(newJlstd_basic_string_view\(jlModule\)\),?\s*$"m => "")

    # 6. Remove export line from Julia module (if present)
    julia_module_file = joinpath(sourcedir, "RDKit", "src", "RDKit.jl")
    if isfile(julia_module_file)
        julia_content = read(julia_module_file, String)
        julia_content = replace(julia_content, r",?\s*std!basic_string_view" => "")
        write(julia_module_file, julia_content)
    end

    # ============================================================
    # General removal of ALL non-RDKit wrappers (std::*, boost::*)
    # ============================================================
    # WrapIt generates wrappers for C++ standard library and boost types
    # that CxxWrap cannot handle at runtime (e.g. "No appropriate factory
    # for type SaIiE" = std::allocator<int>).  We remove every wrapper
    # whose struct name starts with Jlstd_ or Jlboost_.
    #
    # Known instances:
    #   Jlstd_list            -> std::list
    #   Jlstd_map             -> std::map
    #   Jlboost_detail_edge_desc_impl -> boost::detail::edge_desc_impl
    # But this logic catches any future ones automatically.

    # Use a function to avoid Julia soft-scope issues with for-loop
    # variable assignment at top level in scripts.
    function remove_non_rdkit_wrappers!(content::String, julia_module_file::String)
        # Discover all non-RDKit wrapper struct names
        non_rdkit_names = Vector{String}()
        for m in eachmatch(r"\b(Jlstd_[A-Za-z_]\w*|Jlboost_[A-Za-z_]\w*)\b", content)
            name = m.captures[1]
            if !(name in non_rdkit_names)
                push!(non_rdkit_names, name)
            end
        end

        if !isempty(non_rdkit_names)
            println("  Removing non-RDKit wrappers: ", join(non_rdkit_names, ", "))

            # Pass A: For each discovered wrapper name, remove the namespace jlcxx
            # block (if present) that immediately precedes its struct definition.
            # The pattern in the generated code is:
            #   namespace jlcxx {
            #     ... template specializations for std::list<...> ...
            #   }
            #   // Class generating the wrapper for type std::list
            # The namespace block is always a single-level block (no nesting),
            # so [^}]* safely matches the entire block body.
            for wname in non_rdkit_names
                content = replace(content, Regex(
                    "namespace jlcxx \\{[^}]*?" *
                    "(?:std::|boost::)" *
                    "[^}]*\\}\\n"
                , "s") => "")
            end

            # Pass B: For each discovered wrapper name, remove its struct
            # definition block, forward declarations, and factory vector line.
            for wname in non_rdkit_names
                # Remove the struct definition block:
                #   // Class generating the wrapper for type <C++ type>
                #   // signature to use in the veto file: <C++ type>
                #   struct Jlstd_xxx: public Wrapper { ... };
                #   std::shared_ptr<Wrapper> newJlstd_xxx(jlcxx::Module& module){
                #     return ...;
                #   }
                content = replace(content, Regex(
                    "// Class generating the wrapper for type (?:std|boost)[^\n]*\n" *
                    "// signature to use in the veto file: (?:std|boost)[^\n]*\n" *
                    "struct $(wname): public Wrapper \\{[\\s\\S]*?" *
                    "std::shared_ptr<Wrapper> new$(wname)\\(jlcxx::Module& module\\)\\{\\n" *
                    "  return std::shared_ptr<Wrapper>\\(new $(wname)\\(module\\)\\);\\n" *
                    "\\}"
                ) => "")

                # Remove forward class declaration
                content = replace(content, Regex("^class $(wname);\\n", "m") => "")

                # Remove forward function declaration
                content = replace(content, Regex("^std::shared_ptr<Wrapper> new$(wname)\\(jlcxx::Module&\\);\\n", "m") => "")

                # Remove factory line from wrappers vector
                content = replace(content, Regex("^\\s*std::shared_ptr<Wrapper>\\(new$(wname)\\(jlModule\\)\\),?\\s*\$", "m") => "")

                # Remove Julia export line if present (e.g. std!list, boost!detail!edge_desc_impl)
                if isfile(julia_module_file)
                    julia_content = read(julia_module_file, String)
                    # Convert C++ wrapper name to Julia export name:
                    # Jlstd_list -> std!list, Jlboost_detail_edge_desc_impl -> boost!detail!edge_desc_impl
                    julia_export_name = replace(wname[3:end], "_" => "!")
                    # Simple string replace -- no regex metacharacters in export names
                    julia_content = replace(julia_content, ", " * julia_export_name => "")
                    julia_content = replace(julia_content, julia_export_name * ", " => "")
                    julia_content = replace(julia_content, julia_export_name => "")
                    write(julia_module_file, julia_content)
                end
            end
        end
        return content
    end

    content = remove_non_rdkit_wrappers!(content, julia_module_file)

    # ============================================================
    # Remove namespace jlcxx blocks with std::list or std::map BuildParameterList
    # ============================================================
    # Even after vetoing the methods, WrapIt may still emit namespace jlcxx blocks
    # with BuildParameterList<std::list<...>> or BuildParameterList<std::map<...>>
    # specializations. These cause "No appropriate factory" errors at runtime.
    # Wrapped in a function to avoid Julia soft-scope issues with for-loop variable
    # assignment at top level in scripts.
    function remove_std_container_blocks!(content::String)
        # Remove namespace jlcxx blocks that contain BuildParameterList for std::list or std::map.
        # Strategy: split content into lines, find "namespace jlcxx {" opening lines, track brace
        # depth to find the matching "}", then check if the block body contains the marker.
        # This avoids regex backtracking issues on large files.
        lines = split(content, '\n'; keepempty=true)
        markers = ["BuildParameterList<std::list", "BuildParameterList<std::map", "boost::"]
        to_delete = falses(length(lines))

        i = 1
        while i <= length(lines)
            line = lines[i]
            if occursin("namespace jlcxx {", line) && !occursin("}", line)
                # Found opening of a namespace jlcxx block.
                # Track brace depth to find the closing line.
                start_line = i
                depth = count('{', line) - count('}', line)
                end_line = i
                j = i + 1
                while j <= length(lines) && depth > 0
                    depth += count('{', lines[j]) - count('}', lines[j])
                    if depth <= 0
                        end_line = j
                        break
                    end
                    j += 1
                end

                if depth == 0
                    # Collect the block text and check for markers
                    block_text = join(lines[start_line:end_line], '\n')
                    should_remove = false
                    for marker in markers
                        if occursin(marker, block_text)
                            should_remove = true
                            break
                        end
                    end
                    if should_remove
                        for k in start_line:end_line
                            to_delete[k] = true
                        end
                    end
                end
                i = end_line + 1
            else
                i += 1
            end
        end

        # Rebuild content, skipping deleted lines
        new_lines = String[]
        for idx in 1:length(lines)
            if !to_delete[idx]
                push!(new_lines, lines[idx])
            end
        end
        return join(new_lines, '\n')
    end

    content = remove_std_container_blocks!(content)

    if length(content) != original_len
        write(generated_cxx, content)
        println("  Patched generated code (RDProps inheritance + non-RDKit wrapper removal)")
    else
        println("  No patches needed")
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
