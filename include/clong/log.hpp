#ifndef CLONG_LOG_HPP
#define CLONG_LOG_HPP

#include <string>
#include <spdlog/spdlog.h>
#include <clong/format.hpp>
#include <clong/PrettyPrinter.hpp>

namespace clong {
namespace log {

// Bring everything to log
using namespace ::spdlog;

/// Prettify full path to the minimal for useful log
std::string pretty_filename(std::string const& f) {
  auto to_match = std::string("clong/");
  auto i = f.size() - 1;
  for (; i >= 0; i--) {
    if (f.compare(i, to_match.size(), to_match) == 0) {
      break;
    }
  }
  return f.substr(i);
}

/// Colorize formatted arguments with given color
template <typename Color, typename... Args>
std::string colorize(Color const& color, Args&&... args) {
  return ::fmt::format(::fmt::fg(color), std::forward<Args>(args)...);
}

/// Use color representation of the given decl
std::string colored(const clang::Decl* decl) {
  return colorize(::fmt::color::blue_violet, PrettyPrinter::pprint(decl));
}

/// For convenience
#define CLONG_LOG(level, ...) \
  ::clong::log::level(::clong::format("{}:{}:{}: ", \
        ::clong::log::pretty_filename(__FILE__), __FUNCTION__, __LINE__) + __VA_ARGS__);
/**/

}
}

#endif
