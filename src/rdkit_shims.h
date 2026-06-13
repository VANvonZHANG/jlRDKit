#pragma once
// rdkit_shims.h — Hand-written shim functions
// Handle RDKit API patterns that WrapIt cannot auto-generate:
//   1. unique_ptr return values → release to raw pointer for CxxWrap management
//   2. Complex parameter structs (SmilesParserParams, SmilesWriteParams)
//   3. Overload disambiguation

#include <string>
#include <memory>
#include <vector>

#include <GraphMol/ROMol.h>
#include <GraphMol/RWMol.h>
#include <GraphMol/Atom.h>
#include <GraphMol/Bond.h>

// SMILES I/O Shims
std::shared_ptr<RDKit::RWMol> smiles_to_mol(const std::string& smi);
std::string mol_to_smiles(const std::shared_ptr<RDKit::RWMol>& mol);

// Atom traversal shim (returns atom pointer vector, bypassing boost iterators)
std::vector<const RDKit::Atom*> mol_get_atoms(const RDKit::ROMol& mol);

// Bond traversal shim (same pattern as mol_get_atoms)
std::vector<const RDKit::Bond*> mol_get_bonds(const RDKit::ROMol& mol);

// --- File I/O shims ---
std::string mol_to_molblock(const std::shared_ptr<RDKit::RWMol>& mol);
std::shared_ptr<RDKit::RWMol> molblock_to_mol(const std::string& block);
std::vector<std::shared_ptr<RDKit::RWMol>> read_sdf_mols(const std::string& filename);
