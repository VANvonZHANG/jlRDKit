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

// 7. debugMol method takes std::ostream& (blocked by ostream veto above,
//    but also veto the method itself for clarity)
/.*debugMol.*/
