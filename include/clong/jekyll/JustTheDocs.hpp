#ifndef CLONG_JEKYLL_JUSTTHEDOCS_HPP
#define CLONG_JEKYLL_JUSTTHEDOCS_HPP

#include <clong/config.hpp>
#include <clong/fs.hpp>
#include <clong/jekyll/Template.hpp>

namespace clong {
namespace jekyll {

class JustTheDocs : public Template<JustTheDocs> {
  public:
  static void write(std::string const& dst_dir, Context const& ctxt) {
    auto src = make_src_path("just-the-docs");
    auto dst = make_dst_path(dst_dir);

    // Clear
    fs::remove_all(dst);
    // Copy default template content
    fs::copy(src, dst, fs::copy_options::recursive);
    // Create functions refs
    {
      auto path = make_dst_path(dst, "/refs/function/");
      fs::create_directories(path);
      for (auto* f : ctxt.functions()) {
        // Get friendly name
        auto name = ((clang::FunctionDecl*)f->decl)->getNameAsString();
        make_md(fs::path(path).concat(name + ".md"), f->comment);
      }
    }
    // TODO: Other refs
  }
};

}
}

#endif
