#include "lib/clong_test.hpp"

TEST(Test, Multiline) {
  clong::test_temp_file input("test.cpp",
    "/// This is a\n"
    "/// multiline\n"
    "/// comment!\n"
    "void test();\n"
    );

  clong::test({input.path()}, [](clong::Context& ctxt) {
    auto test = *ctxt.functions().begin();
    ASSERT_EQ(test->comment, " This is a\n multiline\n comment!\n");
  });
}
