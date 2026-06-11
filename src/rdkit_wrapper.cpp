/**
 * rdkit_wrapper.cpp — jlRDKit Phase 0: 最小 CxxWrap 桥接
 *
 * 注册 3 个 C++ 类 (Atom, ROMol, RWMol) + 2 个 Shim 自由函数，
 * 证明 Julia 能通过 CxxWrap 直接操作 RDKit 的 C++ 对象。
 */

#include <string>
#include <memory>

// RDKit 头文件
#include <GraphMol/ROMol.h>
#include <GraphMol/RWMol.h>
#include <GraphMol/Atom.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>

// CxxWrap 头文件
#include <jlcxx/jlcxx.hpp>
#include <jlcxx/stl.hpp>

using namespace RDKit;

// 声明 RWMol 的父类关系（CxxWrap 要求）
namespace jlcxx {
template<> struct SuperType<RDKit::RWMol> { typedef RDKit::ROMol type; };
}

// ============================================================
// Shim 函数：简化 RDKit API 为 CxxWrap 可处理的签名
// ============================================================

/// 从 SMILES 创建分子（使用 v2 API）
RWMol* smiles_to_mol(const std::string& smi) {
    RDKit::v2::SmilesParse::SmilesParserParams params;
    params.sanitize = true;
    params.removeHs = true;
    auto ptr = RDKit::v2::SmilesParse::MolFromSmiles(smi, params);
    return ptr.release();
}

/// 分子转 SMILES（MolToSmiles 在 RDKit 命名空间中，不在 SmilesWrite 中）
std::string mol_to_smiles(const RDKit::ROMol& mol) {
    SmilesWriteParams params;
    params.doIsomericSmiles = true;
    params.canonical = true;
    return RDKit::MolToSmiles(mol, params);
}

/// 获取原子数（消除 getNumAtoms(bool) 重载歧义）
unsigned int mol_num_atoms(const RDKit::ROMol& mol) {
    return mol.getNumAtoms();
}

/// 获取键数（消除 getNumBonds(bool) 重载歧义）
unsigned int mol_num_bonds(const RDKit::ROMol& mol) {
    return mol.getNumBonds();
}

/// 获取指定索引的 Atom 指针
const RDKit::Atom* mol_get_atom(const RDKit::ROMol& mol, unsigned int idx) {
    return mol.getAtomWithIdx(idx);
}

// ============================================================
// CxxWrap 模块注册
// ============================================================

JLCXX_MODULE define_julia_module(jlcxx::Module& mod) {
    // --- 注册 Atom 类 ---
    mod.add_type<RDKit::Atom>("Atom")
       .method("getSymbol",    &RDKit::Atom::getSymbol)
       .method("getAtomicNum", &RDKit::Atom::getAtomicNum)
       .method("getIdx",       &RDKit::Atom::getIdx);

    // --- 注册 ROMol 类（通过 lambda 注册自由函数作为方法）---
    mod.add_type<RDKit::ROMol>("ROMol")
       .method("getNumAtoms", [](const RDKit::ROMol& mol) {
           return mol.getNumAtoms();
       })
       .method("getNumBonds", [](const RDKit::ROMol& mol) {
           return mol.getNumBonds();
       })
       .method("getAtomWithIdx", [](const RDKit::ROMol& mol, unsigned int idx) {
           return mol.getAtomWithIdx(idx);
       });

    // --- 注册 RWMol 类（继承 ROMol）---
    mod.add_type<RDKit::RWMol>("RWMol", jlcxx::julia_base_type<RDKit::ROMol>());

    // --- 注册自由函数 ---
    mod.method("smiles_to_mol",  &smiles_to_mol);
    mod.method("mol_to_smiles",  &mol_to_smiles);
}
