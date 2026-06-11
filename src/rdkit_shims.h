#pragma once
// rdkit_shims.h — 手写的 Shim 函数
// 处理 WrapIt 无法自动生成的 RDKit API 模式：
//   1. unique_ptr 返回值 → 释放为裸指针交给 CxxWrap 管理
//   2. 复杂参数结构体（SmilesParserParams, SmilesWriteParams）
//   3. 重载消歧

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

// 原子遍历 Shim（返回原子指针向量，绕过 boost 迭代器）
std::vector<const RDKit::Atom*> mol_get_atoms(const RDKit::ROMol& mol);
