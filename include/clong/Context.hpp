#ifndef CLONG_CONTEXT_HPP
#define CLONG_CONTEXT_HPP

#include <clong/config.hpp>
#include <clong/Node.hpp>
#include <clong/PrettyPrinter.hpp>
#include <unordered_set>
#include <unordered_map>
#include <memory>

namespace clong {

/// Represents a documentation parsed context (with all parsed nodes)
class Context {
  RootNode m_root;
  std::unordered_map<const clang::Decl*, std::unique_ptr<Node>> m_decl2node;
  std::unordered_set<const clang::Decl*> m_visited;
  std::unordered_set<const Node*> m_functions;

  public:
  const RootNode& root() const { return m_root; }
  const std::unordered_set<const Node*>& functions() const { return m_functions; }

  public:
  clang::comments::FullComment* comments_of(const clang::Decl* decl) const {
    // Extract comment node from decl
    return decl->getASTContext().getLocalCommentForDeclUncached(decl);
  }

  bool has_registered_node(const clang::Decl* decl) const {
    return m_decl2node.find(decl) != m_decl2node.end();
  }

  bool has_been_visited(const clang::Decl* decl) const {
    return m_visited.find(decl) != m_visited.end();
  }

  bool has_been_visited_and_registered(const clang::Decl* decl) const {
    return has_been_visited(decl) && has_registered_node(decl);
  }

  void mark_as_visited(const clang::Decl* decl) {
    CLONG_LOG(debug, log::colored(decl));
    m_visited.insert(decl);
  }

  Node* register_node(const clang::Decl* decl) {
    // If already marked as visited, don't go further
    if (has_been_visited(decl)) {
      // If there is a registered node, returns it, otherwise it means that the current decl
      // has been marked as visited and means we cannot register a node to this decl. In this
      // case returns nullptr.
      if (has_registered_node(decl)) {
        return m_decl2node[decl].get();
      } else {
        return nullptr;
      }
    }
    // Visit
    mark_as_visited(decl);
    // Register
    m_decl2node.emplace(decl, std::move(std::make_unique<Node>()));
    auto* node = m_decl2node[decl].get();
    // Save decl
    node->decl = decl;
    // Dump comment
    node->comment = PrettyPrinter::pprint_comments(comments_of(decl));
    // Find parents
    auto walking_decl = decl;
    clang::ASTContext& ast_ctxt = decl->getASTContext();
    while (true) {
      const auto& parents = ast_ctxt.getParents(*walking_decl);
      if (parents.empty()) {
        break;
      }
      walking_decl = parents[0].get<clang::Decl>();
      if (!walking_decl) {
        break;
      }
      // Found a registered parent
      if (has_been_visited_and_registered(walking_decl)) {
        node->parent = m_decl2node[walking_decl].get();
        break;
      }
    }
    // If no parent found, use root as parent node
    if (!node->parent) {
      node->parent = &m_root;
    }
    // Make sure to add children to the parent
    node->parent->children.push_back(node);
    return node;
  }

  void register_node(const clang::Decl* decl, std::unordered_set<const Node*>& set) {
    auto node = register_node(decl);
    // Might be null if the decl has been marked visited but has no registered node
    if (node) {
      set.insert(node);
    }
  }

  void register_function_node(const clang::Decl* decl) {
    register_node(decl, m_functions);
  }
};

}

#endif
