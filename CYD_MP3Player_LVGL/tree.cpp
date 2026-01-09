//================================================================================
// N-ary Tree for directory tree
// https://www.geeksforgeeks.org/dsa/tree-data-structure/
// https://www.geeksforgeeks.org/dsa/generic-treesn-array-trees/
//================================================================================
#include "tree.hpp"

//----------------------------------------------------------------------
// Instantiate static member variables
//----------------------------------------------------------------------
bool        Node::m_found;
Node*       Node::m_found_node;
std::string Node::m_path;
uint32_t    Node::n_depth;
uint32_t    Node::n_nodes;
uint32_t    Node::n_leafs;
uint32_t    Node::n_audio;