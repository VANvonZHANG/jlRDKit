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
#include <GraphMol/Fingerprints/MorganFingerprints.h>
#include <GraphMol/Fingerprints/Fingerprints.h>
#include <GraphMol/MolDraw2D/MolDraw2DSVG.h>
#include <DataStructs/ExplicitBitVect.h>

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

// --- Fingerprint shims ---
// Note: ExplicitBitVect::toString() returns RDKit's binary PICKLE format
// (e.g. 20 bytes for a 2048-bit vector), NOT raw packed bits. This RDKit
// version has no getNumBytes()/getBytes() on ExplicitBitVect, but it does
// expose getNumBits() and getBit(i) (from the base BitVect). We pack the
// raw bits into a uint8_t vector for transport to Julia.
std::vector<uint8_t> calc_morgan_fp(const std::shared_ptr<RDKit::RWMol>& mol,
                                     unsigned int radius, unsigned int nbits) {
    try {
        std::unique_ptr<ExplicitBitVect> fp(
            RDKit::MorganFingerprints::getFingerprintAsBitVect(*mol, radius, nbits));
        unsigned int nbits_actual = fp->getNumBits();
        std::vector<uint8_t> bytes((nbits_actual + 7) / 8, 0);
        for (unsigned int i = 0; i < nbits_actual; ++i) {
            if (fp->getBit(i)) bytes[i / 8] |= static_cast<uint8_t>(1u << (i % 8));
        }
        return bytes;
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("RDKit error: ") + e.what());
    }
}

std::vector<uint8_t> calc_rdkit_fp(const std::shared_ptr<RDKit::RWMol>& mol,
                                    unsigned int min_path, unsigned int max_path) {
    try {
        std::unique_ptr<ExplicitBitVect> fp(
            RDKit::RDKFingerprintMol(*mol, min_path, max_path));
        unsigned int nbits_actual = fp->getNumBits();
        std::vector<uint8_t> bytes((nbits_actual + 7) / 8, 0);
        for (unsigned int i = 0; i < nbits_actual; ++i) {
            if (fp->getBit(i)) bytes[i / 8] |= static_cast<uint8_t>(1u << (i % 8));
        }
        return bytes;
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("RDKit error: ") + e.what());
    }
}

// --- Drawing shim ---
// Note: MolDraw2DSVG has no getSVG(); the internal-stringstream ctor stores
// output and exposes it via getDrawingText().
std::string mol_to_svg(const std::shared_ptr<RDKit::RWMol>& mol,
                        int width, int height) {
    try {
        RDKit::MolDraw2DSVG drawer(width, height);
        drawer.drawMolecule(*mol);
        drawer.finishDrawing();
        return drawer.getDrawingText();
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("RDKit error: ") + e.what());
    }
}
