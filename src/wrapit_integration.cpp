// wrapit_integration.cpp — Bridges WrapIt auto-generated code + hand-written shim functions
//
// Mechanism:
//   1. jlRDKit.cxx is compiled with -Ddefine_julia_module=wrapit_module
//      (set via set_source_files_properties in CMakeLists.txt)
//   2. This file declares wrapit_module as extern, then defines the real
//      define_julia_module that calls wrapit_module first, then adds shims.
//
// WrapIt-generated code registers: 29 types, enums, constants, ~400+ methods
// Shim functions register: smiles_to_mol, mol_to_smiles, mol_get_atoms,
//   mol_get_bonds, mol_to_molblock, molblock_to_mol, read_sdf_mols,
//   calc_morgan_fp, calc_rdkit_fp, mol_to_svg

#include <jlcxx/jlcxx.hpp>
#include <jlcxx/stl.hpp>

#include "rdkit_shims.h"

// Forward-declare the renamed WrapIt entry point
// (jlRDKit.cxx defines this with -Ddefine_julia_module=wrapit_module)
extern "C" void wrapit_module(jlcxx::Module& mod);

// Define the real CxxWrap entry point
JLCXX_MODULE define_julia_module(jlcxx::Module& mod) {
    // Phase 1: Register all WrapIt-generated types, methods, enums, constants
    wrapit_module(mod);

    // Phase 2: Register hand-written shim functions
    // These handle patterns WrapIt cannot generate:
    //   - unique_ptr returns (smiles_to_mol)
    //   - complex parameter structs (mol_to_smiles)
    //   - boost iterator bypass (mol_get_atoms)
    mod.method("smiles_to_mol",  &smiles_to_mol);
    mod.method("mol_to_smiles",  &mol_to_smiles);
    mod.method("mol_get_atoms",  &mol_get_atoms);

    // Bond traversal + File I/O shims
    mod.method("mol_get_bonds",  &mol_get_bonds);
    mod.method("mol_to_molblock",  &mol_to_molblock);
    mod.method("molblock_to_mol",  &molblock_to_mol);
    mod.method("read_sdf_mols",  &read_sdf_mols);

    // Fingerprint + drawing shims
    mod.method("calc_morgan_fp",  &calc_morgan_fp);
    mod.method("calc_rdkit_fp",  &calc_rdkit_fp);
    mod.method("mol_to_svg",  &mol_to_svg);
}
