// rdkit_module.cpp — Manual registration entry point
// Integrates WrapIt auto-generated code reference + hand-written shim functions
// Phase 1: Extended Phase 0 registration with Bond, enums, and more methods

#include <string>
#include <memory>
#include <vector>

// RDKit headers
#include <GraphMol/Atom.h>
#include <GraphMol/Bond.h>
#include <GraphMol/ROMol.h>
#include <GraphMol/RWMol.h>

// CxxWrap headers
#include <jlcxx/jlcxx.hpp>
#include <jlcxx/stl.hpp>

#include "rdkit_shims.h"

// Declare RWMol parent class relationship (required by CxxWrap)
namespace jlcxx {
template<> struct SuperType<RDKit::RWMol> { typedef RDKit::ROMol type; };
}

// ============================================================
// CxxWrap module registration
// ============================================================

JLCXX_MODULE define_julia_module(jlcxx::Module& mod) {
    // --- Register Atom class ---
    mod.add_type<RDKit::Atom>("Atom")
       .method("getSymbol",        [](const RDKit::Atom& a) { return a.getSymbol(); })
       .method("getAtomicNum",     [](const RDKit::Atom& a) { return a.getAtomicNum(); })
       .method("getIdx",           [](const RDKit::Atom& a) { return a.getIdx(); })
       .method("getIsAromatic",    [](const RDKit::Atom& a) { return a.getIsAromatic(); })
       .method("getFormalCharge",  [](const RDKit::Atom& a) { return a.getFormalCharge(); })
       .method("getNumExplicitHs", [](const RDKit::Atom& a) { return a.getNumExplicitHs(); });

    // --- Register Bond class (must be before ROMol because ROMol::getBondWithIdx returns Bond*) ---
    mod.add_type<RDKit::Bond>("Bond")
       .method("getBeginAtomIdx", [](const RDKit::Bond& b) { return b.getBeginAtomIdx(); })
       .method("getEndAtomIdx",   [](const RDKit::Bond& b) { return b.getEndAtomIdx(); })
       .method("getIsAromatic",   [](const RDKit::Bond& b) { return b.getIsAromatic(); })
       .method("getIdx",          [](const RDKit::Bond& b) { return b.getIdx(); });

    // --- Register ROMol class ---
    mod.add_type<RDKit::ROMol>("ROMol")
       .method("getNumAtoms",      [](const RDKit::ROMol& m) { return m.getNumAtoms(); })
       .method("getNumBonds",      [](const RDKit::ROMol& m) { return m.getNumBonds(); })
       .method("getNumHeavyAtoms", [](const RDKit::ROMol& m) { return m.getNumHeavyAtoms(); })
       .method("getAtomWithIdx",   [](const RDKit::ROMol& m, unsigned int i) { return m.getAtomWithIdx(i); })
       .method("getBondWithIdx",   [](const RDKit::ROMol& m, unsigned int i) { return m.getBondWithIdx(i); })
       .method("getBondBetweenAtoms", [](const RDKit::ROMol& m, unsigned int a, unsigned int b) { return m.getBondBetweenAtoms(a, b); })
       .method("getNumConformers", [](const RDKit::ROMol& m) { return m.getNumConformers(); });

    // --- Register RWMol class (inherits ROMol) ---
    mod.add_type<RDKit::RWMol>("RWMol", jlcxx::julia_base_type<RDKit::ROMol>());

    // --- Register enum types ---
    // Note: add_enum internally calls add_bits, so no separate add_bits call needed
    // Enum values prefixed to avoid name collisions across different enums (e.g., UNSPECIFIED appears in multiple enums)

    // BondType
    mod.add_enum<RDKit::Bond::BondType, int>("BondType",
        {"BT_UNSPECIFIED", "BT_SINGLE", "BT_DOUBLE", "BT_TRIPLE", "BT_QUADRUPLE", "BT_QUINTUPLE",
         "BT_HEXTUPLE", "BT_ONEANDAHALF", "BT_TWOANDAHALF", "BT_THREEANDAHALF",
         "BT_FOURANDAHALF", "BT_FIVEANDAHALF", "BT_AROMATIC", "BT_IONIC", "BT_HYDROGEN",
         "BT_THREECENTER", "BT_DATIVEONE", "BT_DATIVE", "BT_DATIVEL", "BT_DATIVER", "BT_OTHER",
         "BT_ZERO"},
        {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
         19, 20, 21});

    // BondDir
    mod.add_enum<RDKit::Bond::BondDir, int>("BondDir",
        {"BD_NONE", "BD_BEGINWEDGE", "BD_BEGINDASH", "BD_ENDDOWNRIGHT", "BD_ENDUPRIGHT",
         "BD_EITHERDOUBLE", "BD_UNKNOWN"},
        {0, 1, 2, 3, 4, 5, 6});

    // BondStereo
    mod.add_enum<RDKit::Bond::BondStereo, int>("BondStereo",
        {"BST_STEREONONE", "BST_STEREOANY", "BST_STEREOZ", "BST_STEREOE", "BST_STEREOCIS",
         "BST_STEREOTRANS", "BST_STEREOATROPCW", "BST_STEREOATROPCCW"},
        {0, 1, 2, 3, 4, 5, 6, 7});

    // HybridizationType
    mod.add_enum<RDKit::Atom::HybridizationType, int>("HybridizationType",
        {"HYB_UNSPECIFIED", "HYB_S", "HYB_SP", "HYB_SP2", "HYB_SP3", "HYB_SP2D", "HYB_SP3D", "HYB_SP3D2",
         "HYB_OTHER"},
        {0, 1, 2, 3, 4, 5, 6, 7, 8});

    // ChiralType
    mod.add_enum<RDKit::Atom::ChiralType, int>("ChiralType",
        {"CHI_UNSPECIFIED", "CHI_TETRAHEDRAL_CW", "CHI_TETRAHEDRAL_CCW",
         "CHI_OTHER", "CHI_TETRAHEDRAL", "CHI_ALLENE", "CHI_SQUAREPLANAR",
         "CHI_TRIGONALBIPYRAMIDAL", "CHI_OCTAHEDRAL"},
        {0, 1, 2, 3, 4, 5, 6, 7, 8});

    // --- Register Bond::getBondType (after BondType enum, returns int via shim) ---
    mod.method("getBondType", [](const RDKit::Bond& b) -> int {
        return static_cast<int>(b.getBondType());
    });

    // --- Register free functions (shim functions) ---
    mod.method("smiles_to_mol",  &smiles_to_mol);
    mod.method("mol_to_smiles",  &mol_to_smiles);
    mod.method("mol_get_atoms",  &mol_get_atoms);
}
