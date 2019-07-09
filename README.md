Clong
=====

A basic documentation generator implemented using libtooling from `clang` and `jekyll` engine.

Getting started
===============

You first need to compile `clong`. As usual, create a directory to build using cmake:
```
mkdir build
cd build
cmake ..
make
```

> If you want to install it locally, you can add the following cmake option
> `-DCMAKE_INSTALL_PREFIX="$HOME/.local"`.

Install `clong`:
```
make install
```

- - -

Now it's time to generate something! Here's how:

```
clong <file.cpp>
```

By default, it will create a new folder named `_doc`. To explore the generated document, move to
you doc folder and start serving the doc:

```
bundle exec jekyll serve
```

You now browse you [documentation](http://localhost:4000). Enjoy.
