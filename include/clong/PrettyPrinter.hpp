#ifndef CLONG_PRETTYPRINTER_HPP
#define CLONG_PRETTYPRINTER_HPP

#include <clong/config.hpp>
#include <clong/clang.hpp>
#include <clong/Node.hpp>

namespace clong {

/// Pretty printer utils
class PrettyPrinter {
  private:
  static bool ends_with(std::string const& str, std::string const& with) {
    if (with.size() > str.size()) {
      return false;
    }
    return std::equal(with.rbegin(), with.rend(), str.rbegin());
  }

  public:
  static std::string pprint(const clang::Decl* decl, clang::LangOptions const& lo) {
    assert(decl && "`decl` cannot be null");
    clang::PrintingPolicy pp(lo);
    // Skip body pprinting (function + class bodies)
    pp.TerseOutput = true;
    // Create steam and buffer
    std::string s;
    llvm::raw_string_ostream ss(s);
    // Extract pretty representation of the decl
    decl->print(ss, pp);
    // Force flush of stream
    s = ss.str();
    // Post-process pprinted buffer
    s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
    // Check for " {}"
    std::string bad_brackets = " {}";
    if (ends_with(s, bad_brackets)) {
      s.erase(s.end() - bad_brackets.length(), s.end());
    }
    return s;
  }

  static std::string pprint(const clang::Decl* decl) {
    clang::LangOptions lo;
    return pprint(decl, lo);
  }

  static std::string ascii_encode(std::string const& s) {
    std::string e;
    for (char c : s) {
      if (c == '\n') {
        e += "\\n";
      } else {
        e += c;
      }
    }
    return e;
  }

  static std::string pprint(const RootNode* node) {
    std::string p;
    for (auto const* child : node->children) {
      p += pprint(child);
    }
    return p;
  }

  static std::string pprint(const Node* node, int level = 0) {
    std::string p;
    std::string prefix = "- ";
    for (int i = 0; i < level; ++i) {
      prefix += "  ";
    }
    p += prefix + pprint(node->decl) + " -- " + ascii_encode(node->comment) + "\n";
    for (auto const* child : node->children) {
      p += pprint(child, level + 1);
    }
    return p;
  }

  static std::string pprint_comments(const clang::comments::Comment* com) {
    using comment_t = clang::comments::TextComment;
    std::string p;
    if (com) {
        if (clang::isa<comment_t>(com)) {
          p += clang::dyn_cast<comment_t>(com)->getText();
          p += "\n";
        }
        for (auto child = com->child_begin(); child != com->child_end(); ++child) {
          p += pprint_comments(*child);
        }
    }
    return p;
  }
};

}

#endif
