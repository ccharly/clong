/// namespace doc
namespace sp {
    /// Foo bro
    void foo() {
    }

    /// test
    enum test {
        OK,
        NOK
    };

    /// Foo
    #define foo()
}

/// G-var doc
int g_var = 10;

/// Test
class omg 
{
    /// Doc
    void bar();

    /// Not doc
    /// Not doc
    /// Not doc
    void not_doc() {
    }

    /// Foobar
    enum ok {
    };
};

///
class inherited : public omg {
};

///
struct pod {
    int a;
    int b;
};

///
template <typename T>
class tpl_inherited : public omg {
};

///
template <>
class tpl_inherited<float> : tpl_inherited<int> {
};

/// tpl
template <typename T>
void tpl() {
}

/// namespace doc
namespace sp {
    /// Bar bro
    void bar() {
    }
}

/// The main
int main() {
    return 0;
}
