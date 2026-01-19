//================================================================================
// N-ary Tree for directory tree
//  Auther: embedded-kiddie (https://github.com/embedded-kiddie)
//  Released under the GPLv3 (https://www.gnu.org/licenses/gpl-3.0.html)
//  https://www.geeksforgeeks.org/dsa/tree-data-structure/
//  https://www.geeksforgeeks.org/dsa/generic-treesn-array-trees/
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