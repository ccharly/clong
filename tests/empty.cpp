#include "lib/clong_test.hpp"

TEST(Test, Empty) {
  clong::test_temp_file input("test.cpp",
    "// This is a comment yes... But not a doc comment!\n"
    "void test();\n"
    "\n"
    "class class_test {\n"
    "  // We're inside the class"
    "\n"
    "  int tested(int code)\n;"
    "};\n"
    );

  clong::test({input.path()}, [](clong::Context& ctxt) {
    ASSERT_EQ(ctxt.root().children.size(), 0);
  });
}
