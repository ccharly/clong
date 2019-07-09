#ifndef CLONG_JEKYLL_HPP
#define CLONG_JEKYLL_HPP

#include "clong.hpp"
#include <ghc/filesystem.hpp>

namespace clong {
namespace jekyll {

namespace fs = ::ghc::filesystem;

template <typename JekyllTemplate>
class Template {
  public:
  static fs::path make_src_path(std::string const& template_dir) {
    return fs::path("../jekyll-template/").concat(template_dir);
  }

  static fs::path make_dst_path(std::string const& dst_dir, std::string const& next_dir = "") {
    return fs::path(dst_dir).concat(next_dir);
  }

  static void make_md(fs::path const& path, std::string const& content) {
    fs::ofstream o(path, fs::ofstream::out);

    o << content;
    o.close();
  }

  static void write(std::string const& dst_dir, Context const& ctxt);
};

class JustTheDocs : public Template<JustTheDocs> {
  public:
  static void write(std::string const& dst_dir, Context const& ctxt) {
    auto src = make_src_path("just-the-docs");
    auto dst = make_dst_path(dst_dir);

    fs::remove_all(dst);
    fs::copy(src, dst, fs::copy_options::recursive);
    {
      auto path = make_dst_path(dst, "/code/function/");
      fs::create_directories(path);
      for (auto* f : ctxt.functions()) {
        // Get friendly name
        std::cout << "f: " << (void*)f << std::endl;
        std::cout << (void*)f->decl << std::endl;
        auto name = ((clang::FunctionDecl*)f->decl)->getNameAsString();
        std::cout << name << std::endl;
        make_md(fs::path(path).concat(name + ".md"), f->comment);
      }
    }
    /*
    fs::create_directories(make_dst_path(dst, "/code/namespace"));
    fs::create_directories(make_dst_path(dst, "/code/class"));
    fs::create_directories(make_dst_path(dst, "/code/struct"));
    fs::create_directories(make_dst_path(dst, "/code/enum"));
    */
  }
};


}
}

#endif
