// rdkit_shims.cpp — Shim function implementations
#include "rdkit_shims.h"
#include <GraphMol/ROMol.h>
#include <GraphMol/RWMol.h>
#include <GraphMol/Atom.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/Bond.h>
#include <GraphMol/FileParsers/FileParsers.h>
#include <GraphMol/FileParsers/MolSupplier.h>

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

// Traverse all bonds, return raw pointer vector (same pattern as mol_get_atoms)
std::vector<const RDKit::Bond*> mol_get_bonds(const RDKit::ROMol& mol) {
    std::vector<const RDKit::Bond*> result;
    for (const auto& bond : mol.bonds()) {
        result.push_back(bond);
    }
    return result;
}

// --- File I/O shims ---
// Note: RDKit v2 API uses MolFromMolBlock (returns unique_ptr), not the
// v1 MolBlockToMol inline (which returns raw pointer + is a thin wrapper).
std::string mol_to_molblock(const std::shared_ptr<RDKit::RWMol>& mol) {
    try {
        return RDKit::MolToMolBlock(*mol);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("RDKit error: ") + e.what());
    }
}

std::shared_ptr<RDKit::RWMol> molblock_to_mol(const std::string& block) {
    try {
        RDKit::v2::FileParsers::MolFileParserParams params;
        params.sanitize = true;
        params.removeHs = true;
        auto mol = RDKit::v2::FileParsers::MolFromMolBlock(block, params);
        return std::shared_ptr<RDKit::RWMol>(mol.release());
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("RDKit error: ") + e.what());
    }
}

std::vector<std::shared_ptr<RDKit::RWMol>> read_sdf_mols(const std::string& filename) {
    try {
        std::vector<std::shared_ptr<RDKit::RWMol>> result;
        auto suppl = RDKit::v2::FileParsers::SDMolSupplier(filename);
        while (!suppl.atEnd()) {
            auto mol = suppl.next();
            if (mol) {
                result.emplace_back(std::shared_ptr<RDKit::RWMol>(mol.release()));
            }
        }
        return result;
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("RDKit error: ") + e.what());
    }
}
