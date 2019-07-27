#include "lib/clong_test.hpp"

TEST(Test, AutoRegisterParent) {
  clong::test_temp_file input("test.cpp",
    "struct not_documented {\n"
    "  /// Is documented\n"
    "  void i_am_documented();\n"
    "};\n"
    );

  clong::test({input.path()}, [](clong::Context& ctxt) {
    auto root = ctxt.root();
    ASSERT_GT(root.children.size(), 0);
    auto not_documented = root.children[0];
    ASSERT_GT(not_documented->children.size(), 0);
    auto i_am_documented = not_documented->children[0];
    ASSERT_EQ(clong::ident(not_documented->decl), "not_documented");
    ASSERT_EQ(clong::ident(i_am_documented->decl), "i_am_documented");
  });
}

TEST(Test, AutoRegisterParentMultiple) {
  clong::test_temp_file input("test.cpp",
    "namespace really_not_documented {\n"
    "  struct not_documented {\n"
    "    /// Is documented\n"
    "    void i_am_documented();\n"
    "  };\n"
    "}\n"
    );

  clong::test({input.path()}, [](clong::Context& ctxt) {
    auto root = ctxt.root();
    ASSERT_GT(root.children.size(), 0);
    auto really_not_documented = root.children[0];
    ASSERT_GT(really_not_documented->children.size(), 0);
    auto not_documented = really_not_documented->children[0];
    ASSERT_GT(not_documented->children.size(), 0);
    auto i_am_documented = not_documented->children[0];
    ASSERT_EQ(clong::ident(really_not_documented->decl), "really_not_documented");
    ASSERT_EQ(clong::ident(not_documented->decl), "not_documented");
    ASSERT_EQ(clong::ident(i_am_documented->decl), "i_am_documented");
  });
}
