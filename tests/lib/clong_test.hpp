#ifndef CLONG_TEST_HPP
#define CLONG_TEST_HPP

#include <clong/clong.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <vector>

// Only for testing purposes
namespace clong {

// Creates a temporary file
class test_temp_file {
  std::string _path;

  public:
  test_temp_file(std::string const& name, std::string const& content) {
    // Get system temporary dir
    auto tmp = clong::fs::temp_directory_path();
    // Concat file name
    tmp += clong::fs::path(name);
    // Save the full path
    _path = tmp;
    // Now write the content
    std::fstream f(tmp, std::fstream::out);
    f << content;
  }

  ~test_temp_file() {
    // Make sure to remove/close the file at the end
    clong::fs::remove_all(_path);
  }

  public:
  std::string const& path() const {
    return _path;
  }
};

CLONG_DLLIMPORT std::string test_read_file(std::string const& path);
CLONG_DLLIMPORT void test(std::vector<std::string> const& args, std::function<void (clong::Context&)> on_end);

}


#endif
