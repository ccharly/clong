#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <clong/clang.hpp>
#include <clong/clong.hpp>
#include <clong/run.hpp>
#include <clong/fs.hpp>

namespace clong {

// From: https://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
// Read a file entirely (might be used for future tests)
std::string test_read_file(std::string const& path) {
  std::string content;
  std::ifstream t(path);

  t.seekg(0, std::ios::end);
  content.reserve(t.tellg());
  t.seekg(0, std::ios::beg);

  content.assign(std::istreambuf_iterator<char>(t), std::istreambuf_iterator<char>());
  return content;
}

// No templates here as we want to link against the test library
void test(std::vector<std::string> const& args, std::function<void (clong::Context&)> on_end) {
  // Set log level to debug
  clong::log::set_level(clong::log::level::debug);
  // Holds "fake" argv arguments
  std::vector<const char*> argv(args.size());
  // Use string pointers for the "fake" argv
  std::transform(std::begin(args), std::end(args), std::begin(argv), [](auto const& x) {
    return x.c_str();
  });
  // Clang relies on having "valid" argv[0], so use a dummy one
  argv.insert(std::begin(argv), "test");
  // Make run silent (otherwise Clang will output some warnings about compilation database...)
  argv.push_back("--");
  // Now we can run `clong`
  clong::run(argv.size(), argv.data(), on_end);
}

}
