// rdkit_shims.cpp — Shim 函数实现
#include "rdkit_shims.h"
#include <GraphMol/ROMol.h>
#include <GraphMol/RWMol.h>
#include <GraphMol/Atom.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>

// SMILES → 分子（隐藏 SmilesParserParams）
std::shared_ptr<RDKit::RWMol> smiles_to_mol(const std::string& smi) {
    RDKit::v2::SmilesParse::SmilesParserParams params;
    params.sanitize = true;
    params.removeHs = true;
    auto ptr = RDKit::v2::SmilesParse::MolFromSmiles(smi, params);
    return std::shared_ptr<RDKit::RWMol>(ptr.release());
}

// 分子 → SMILES（隐藏 SmilesWriteParams）
std::string mol_to_smiles(const std::shared_ptr<RDKit::RWMol>& mol) {
    RDKit::SmilesWriteParams params;
    params.doIsomericSmiles = true;
    params.canonical = true;
    return RDKit::MolToSmiles(*mol, params);
}

// 遍历所有原子，返回裸指针向量（绕过 boost 迭代器）
std::vector<const RDKit::Atom*> mol_get_atoms(const RDKit::ROMol& mol) {
    std::vector<const RDKit::Atom*> result;
    for (const auto& atom : mol.atoms()) {
        result.push_back(atom);
    }
    return result;
}
