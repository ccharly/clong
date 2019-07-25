#ifndef CLONG_NODE_HPP
#define CLONG_NODE_HPP

#include <clong/config.hpp>
#include <clong/clang.hpp>

namespace clong {

/// A node definition with it's decl `clang` declaration
struct Node {
  Node* parent;
  std::string comment;
  const clang::Decl* decl;
  std::vector<Node*> children;
};

/// The root node of an AST
struct RootNode : Node {
};

}

#endif
