#ifndef CLONG_JEKYLL_TEMPLATE_HPP
#define CLONG_JEKYLL_TEMPLATE_HPP

#include <clong/config.hpp>
#include <clong/fs.hpp>

namespace clong {
namespace jekyll {

template <typename JekyllTemplate>
class Template {
  public:
  static fs::path make_src_path(std::string const& template_dir) {
    return fs::path("../jekyll/").concat(template_dir);
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

}
}

#endif
