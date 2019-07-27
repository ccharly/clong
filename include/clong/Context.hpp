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

  Node* register_node(const clang::Decl* decl, bool allow_no_comments = false) {
    // If there is a visited and  registered node, returns it directly!
    if (has_been_visited_and_registered(decl)) {
      return m_decl2node[decl].get();
    }
    // Visit
    mark_as_visited(decl);
    // Extract comment if any, if none, we're not gonna register this node!
    auto comment = PrettyPrinter::pprint_comments(comments_of(decl));
    if (allow_no_comments || comment.size()) {
      // Register
      CLONG_LOG(debug, log::colored(decl));
      m_decl2node.emplace(decl, std::move(std::make_unique<Node>()));
      auto* node = m_decl2node[decl].get();
      // Update node infos
      node->decl = decl;
      node->comment = comment;
      // Walk visited parents and set relationships
      auto* walking_decl = decl;
      auto& ast_ctxt = decl->getASTContext();
      while (true) {
        const auto& parents = ast_ctxt.getParents(*walking_decl);
        // If no parents or the first parent decl is null, then we're done 
        if (parents.empty() || !(walking_decl = parents[0].get<clang::Decl>())) {
          break;
        }
        // Found a visited parent
        if (has_been_visited(walking_decl)) {
          // Calls register to make sure we're registering parents even though they don't
          // have any doc comment!
          // NOTE: It returns node directly if already registered
          bool allow_no_comments = true;
          node->parent = register_node(walking_decl, allow_no_comments);
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
    // Means the node has not been registered!
    return nullptr;
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
