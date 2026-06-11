// gen/veto.h — WrapIt veto list for RDKit
// 排除含有无法解析的模板参数的方法

// 模板查询类（Queries::Query 模板参数无法解析）
/.*Queries::Query.*Atom.*true.*/
/.*QUERYATOM_QUERY.*/
/.*QUERYBOND_QUERY.*/

// boost 图类型（模板嵌套太深）
/.*boost::adjacency_list.*/
/.*boost::graph_traits.*/
/.*MolGraph.*/

// boost 智能指针相关
/.*boost::shared_ptr.*ROMol.*/
/.*boost::dynamic_bitset.*/

// 迭代器类型（boost 迭代器适配器）
/.*AtomIterator_.*/
/.*BondIterator_.*/
/.*CXXAtomIterator.*/
/.*CXXBondIterator.*/

// std::uint64_t 类型问题
/.*getFlags.*/

// 输出流操作符
/.*operator<<.*/
