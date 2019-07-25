#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <clong/clang.hpp>
#include <clong/clong.hpp>
#include <clong/run.hpp>

int main(int argc, const char** argv) {
  // Hook called whenever the tool as finished running
  return clong::run(argc, argv, [](clong::Context& ctxt) {
    // Pretty print current parsed nodes
    std::cout << clong::PrettyPrinter::pprint(&ctxt.root());

    // Output to jekyll format
    clong::jekyll::JustTheDocs::write(clong::OutputDir, ctxt);
  });
}
