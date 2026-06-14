// gen/veto.h — WrapIt veto list for RDKit
// Exclude methods with unresolvable template parameters

// Template query classes (Queries::Query template params cannot be resolved)
/.*Queries::Query.*Atom.*true.*/
/.*QUERYATOM_QUERY.*/
/.*QUERYBOND_QUERY.*/

// Boost graph types (template nesting too deep)
/.*boost::adjacency_list.*/
/.*boost::graph_traits.*/
/.*MolGraph.*/

// Boost smart pointer related
/.*boost::shared_ptr.*ROMol.*/
/.*boost::dynamic_bitset.*/

// Iterator types (boost iterator adapters)
/.*AtomIterator_.*/
/.*BondIterator_.*/
/.*CXXAtomIterator.*/
/.*CXXBondIterator.*/

// std::uint64_t type issues
/.*getFlags.*/

// Output stream operators
/.*operator<<.*/

// ================================================================
// Phase 17 additions: Fix WrapIt compilation errors
// ================================================================

// 1. Function pointer parameters — WrapIt generates invalid lambda syntax
//    Error: expected ',' or '...' before '(' token
//    Broad match: any setAromaticity with function pointer arg
/.*setAromaticity.*\(\*\).*/

// 2. Non-copyable unique_ptr in vector parameters/returns
//    Error: use of deleted function 'unique_ptr<...>::unique_ptr(const unique_ptr<...>&)'
//    getMolFrags with vector<unique_ptr<ROMol>> parameter
/.*getMolFrags.*unique_ptr.*/
//    detectChemistryProblems returns vector<unique_ptr<MolSanitizeException>>
/.*detectChemistryProblems.*/

// 3. v2::SmilesParse functions returning unique_ptr (non-copyable return types)
/.*v2::SmilesParse::MolFromSmiles.*/
/.*v2::SmilesParse::MolFromSmarts.*/
/.*v2::SmilesParse::AtomFromSmiles.*/
/.*v2::SmilesParse::BondFromSmiles.*/
/.*v2::SmilesParse::AtomFromSmarts.*/
/.*v2::SmilesParse::BondFromSmarts.*/

// 4. User-defined literal operators — WrapIt generates broken method names
/.*operator""_smiles.*/
/.*operator""_smarts.*/

// 5. RDProps base class — not polymorphic (no virtual destructor)
//    Causes: cannot dynamic_cast (source type is not polymorphic)
//    and DownCast method resolution failures for Atom/Bond/ROMol/RWMol
//    Veto the entire class to prevent inheritance declarations
/RDKit::RDProps/

// 6. std::basic_ostream — protected default constructor
//    Error: basic_ostream() is protected within this context
/std::basic_ostream/

// 6b. std::basic_istream — CxxWrap has no factory for std::basic_istream<char>
//     (mangled Si). Methods taking std::istream&/std::istream* abort @wrapmodule
//     with "No appropriate factory for type Si".
//     These are all stream-input molecule readers; the filename/block overloads
//     (e.g. MolFromMolFile, MolFromMolBlock, MolFromPDBFile) remain available.
//     API loss: the *DataStream* / *Stream* / initFromStream / molFromPickle(istream)
//     variants are unavailable from Julia — stream I/O is not exposed.
/.*std::istream.*/

// 6c. std::streampos (aka std::fpos<__mbstate_t>, mangled St4fposI11__mbstate_tE) —
//     CxxWrap has no factory for this stream-position type. SDMolSupplier's internal
//     setStreamIndices takes a vector<streampos> to manually position the supplier.
//     This is a low-level indexing helper; the normal SDMolSupplier iteration API
//     (reader at path, mol at index, length, etc.) remains available.
//     API loss: manual stream-offset indexing of SDMolSupplier is unavailable.
/.*SDMolSupplier::setStreamIndices.*/

// 6d. std::pair<unsigned int, unsigned int> (mangled St4pairIjjE) — CxxWrap has no
//     factory for this pair instantiation. Chirality::findMesoCenters returns a
//     vector<pair<uint,uint>> (meso-center atom-index pairs). Other Chirality
//     analysis methods (findStereo, getStereoInfo, assignStereochemistry) remain
//     available.
//     API loss: findMesoCenters (meso-center detection returning atom-index pairs)
//     is unavailable from Julia.
/.*Chirality::findMesoCenters.*/

// 7. debugMol method takes std::ostream& (blocked by ostream veto above,
//    but also veto the method itself for clarity)
/.*debugMol.*/

// 8. Bookmark map types — return std::map<int, std::list<Atom/Bond*>>
//    CxxWrap cannot create factories for these deeply nested std types
/.*getAtomBookmarks.*/
/.*getBondBookmarks.*/

// 9. Boost edge_descriptor — used by ROMol::operator[] but CxxWrap can't handle
//    the boost::detail::edge_desc_impl template
/.*edge_descriptor.*/

// 10. Methods returning/using std::map or std::list — CxxWrap can't handle
//     nested std allocators in these template types.
//     Error: "No appropriate factory for type St3mapI..." (std::map<string,string>)
//     or "No appropriate factory for type St4listI..." (std::list<int>)

// replacements field in SmilesParserParams/SmartsParserParams returns std::map<string,string>
/.*replacements.*/

// SmilesToMol overloads that take std::map<string,string>* parameter
// (the SmilesParserParams overload is fine since params themselves are passed by ref)
/.*SmilesToMol.*std::map.*/

// SmartsToMol overloads that take std::map<string,string>* parameter
/.*SmartsToMol.*std::map.*/

// getShortestPath returns std::list<int>
/.*getShortestPath.*/

// 11. Methods returning/using std::string_view (aka std::basic_string_view)
//     CxxWrap cannot create factories for std::string_view because it requires
//     std::char_traits<char> which is not registered.
//     Error: "No appropriate factory for type St17basic_string_viewIcSt11char_traitsIcEE"

// common_properties constants are all const std::string_view&
/.*common_properties.*/

// detail::computedPropName returns const std::string_view&
/.*detail::computedPropName.*/

// getNumAtomsWithDistinctProperty takes string_view parameter
/.*getNumAtomsWithDistinctProperty.*/

// 11b. std::array of custom types — CxxWrap has no factory for std::array<T, N>
//      when T is a non-trivial (e.g. RDGeom::Point3D) custom type.
//      Error: "No appropriate factory for type St5arrayIN6RDGeom7Point3DELm3EE"
//      (= std::array<RDGeom::Point3D, 3>).
//      SubstanceGroup.h defines:
//        typedef std::array<RDGeom::Point3D, 3> Bracket;
//      and getBrackets() returns std::vector<Bracket>. Since CxxWrap cannot
//      build a factory for std::array<RDGeom::Point3D,3>, registration of
//      getBrackets (4 overloads: const& and non-const& for both ref/ptr this)
//      aborts the entire @wrapmodule call. Vetoing the method name catches all
//      overloads. clearBrackets() returns void and is fine (not vetoed).
//      API loss: SubstanceGroup bracket access (getBrackets) is unavailable
//      from Julia; the underlying C++ bracket data cannot be inspected or
//      mutated through this binding.
/.*SubstanceGroup::getBrackets.*/

// ================================================================
// Batch 1 additions: Fix WrapIt compilation errors for new GraphMol headers
// ================================================================

// 12. Mol2 reader functions (FileParsers.h): Mol2BlockToMol, Mol2FileToMol,
//     Mol2DataStreamToMol. WrapIt generates lambda bodies that reference the
//     enum as unqualified `Mol2Type` (e.g. `RDKit::Mol2BlockToMol(arg0, arg1,
//     arg2, arg3, arg4)` where arg3 is `Mol2Type`) but the enum lives at
//     `RDKit::v2::FileParsers::Mol2Type`. The unqualified name fails with
//     "'Mol2Type' has not been declared". Vetoing the signature vetoes the
//     whole overload group (WrapIt groups all overloads under one signature
//     comment). The std::istream overloads couldn't be wrapped anyway; the
//     block/file overloads are a documented loss.
//     Error: 'Mol2Type' has not been declared / invalid conversion from 'int'
//     Note: WrapIt matches the qualified signature "<ret> RDKit::Mol2BlockToMol(...)"
//     (free function in namespace RDKit, no leading ::). The leading space matches the
//     `<ret-type> RDKit::Mol2BlockToMol(...)` free-function signature form, which is why
//     `::RDKit::...` references in other declarations don't need matching. The v2 API
//     (RDKit::v2::FileParsers::MolFromMol2Block / MolFromMol2File / MolFromMol2DataStream)
//     is NOT vetoed and remains available, mitigating the loss.
/.* RDKit::Mol2BlockToMol.*/
/.* RDKit::Mol2FileToMol.*/
/.* RDKit::Mol2DataStreamToMol.*/

// 13. Chirality env-var globals are `constexpr auto = "..."` (deduced type
//     const char*). CxxWrap renders this as `const char *const` (a const
//     pointer to const char), for which no factory is registered, so WrapIt
//     aborts with "Failed to find type declaration of const char *const".
//     These are environment-variable name strings (not chemistry API);
//     vetoing them is a trivial loss.
//     Error: Failed to find type declaration of const char *const
/.*Chirality::nonTetrahedralStereoEnvVar.*/
/.*Chirality::useLegacyStereoEnvVar.*/

// 14. Chirality::useLegacyStereoPerception is an `extern bool` global in the
//     header but no RDKit_jll shared library actually exports the data symbol
//     (verified via nm -D across all libRDKit*.so). The constexpr siblings
//     (nonTetrahedralStereoDefaultVal, useLegacyStereoDefaultVal,
//     minRingSizeForDoubleBondStereo, StereoInfo::NOATOM) fold at compile
//     time and are fine; this one is a true `extern` and produces an
//     undefined-symbol error at dlopen time. This global is surfaced for wrapping
//     because `fields_and_variables = true` in config.toml. The get/set accessors
//     for the same setting (getUseLegacyStereoPerception /
//     setUseLegacyStereoPerception) are wrapped and remain available.
//     Error: undefined symbol: _ZN5RDKit9Chirality25useLegacyStereoPerceptionE
/.*Chirality::useLegacyStereoPerception.*/

// 15. MolSupplier::length() — the three concrete MolSupplier subclasses
//     (SDMolSupplier, SmilesMolSupplier, TDTMolSupplier) each define a C++
//     `unsigned int length()` returning the molecule count. WrapIt registers
//     these as a Julia method literally named `length`. After `@wrapmodule`,
//     this method collides with `Base.length`, and CxxWrap's method-resolution
//     ends up shadowing `Base.length` for *all* collection types (StdVector,
//     Vector{UInt8}, etc.) — not just MolSupplier — so `length(v)` fails with a
//     MethodError on every array. This is pervasive and breaks the fingerprint
//     shim (which returns a StdVector{UInt8}) and the atom-traversal shim
//     (StdVector of Atom). Vetoing the `length` methods is the clean fix.
//     API loss: the molecule-count helper on mol suppliers is unavailable from
//     Julia; users iterate via next()/atEnd() instead (read_sdf_mols shim
//     returns a Julia Vector of mols, so its length is unaffected).
/.*FileParsers::SDMolSupplier::length.*/
/.*FileParsers::SmilesMolSupplier::length.*/
/.*FileParsers::TDTMolSupplier::length.*/
