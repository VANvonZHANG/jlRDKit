#pragma once
// rdkit_shims.h — Hand-written shim functions
// Handle RDKit API patterns that WrapIt cannot auto-generate:
//   1. unique_ptr return values → release to raw pointer for CxxWrap management
//   2. Complex parameter structs (SmilesParserParams, SmilesWriteParams)
//   3. Overload disambiguation

#include <string>
#include <memory>
#include <vector>

namespace RDKit {
    class RWMol;
    class ROMol;
    class Atom;
}

// SMILES I/O Shims
std::shared_ptr<RDKit::RWMol> smiles_to_mol(const std::string& smi);
std::string mol_to_smiles(const std::shared_ptr<RDKit::RWMol>& mol);

// Atom traversal shim (returns atom pointer vector, bypassing boost iterators)
std::vector<const RDKit::Atom*> mol_get_atoms(const RDKit::ROMol& mol);
