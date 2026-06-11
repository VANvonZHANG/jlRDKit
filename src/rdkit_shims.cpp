// rdkit_shims.cpp — Shim function implementations
#include "rdkit_shims.h"
#include <GraphMol/ROMol.h>
#include <GraphMol/RWMol.h>
#include <GraphMol/Atom.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>

// SMILES to molecule (hides SmilesParserParams)
std::shared_ptr<RDKit::RWMol> smiles_to_mol(const std::string& smi) {
    RDKit::v2::SmilesParse::SmilesParserParams params;
    params.sanitize = true;
    params.removeHs = true;
    auto ptr = RDKit::v2::SmilesParse::MolFromSmiles(smi, params);
    return std::shared_ptr<RDKit::RWMol>(ptr.release());
}

// Molecule to SMILES (hides SmilesWriteParams)
std::string mol_to_smiles(const std::shared_ptr<RDKit::RWMol>& mol) {
    RDKit::SmilesWriteParams params;
    params.doIsomericSmiles = true;
    params.canonical = true;
    return RDKit::MolToSmiles(*mol, params);
}

// Traverse all atoms, return raw pointer vector (bypassing boost iterators)
std::vector<const RDKit::Atom*> mol_get_atoms(const RDKit::ROMol& mol) {
    std::vector<const RDKit::Atom*> result;
    for (const auto& atom : mol.atoms()) {
        result.push_back(atom);
    }
    return result;
}
