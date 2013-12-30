header-expander
===============

Provide a simple header-expander for C++ classes written with libclang

Compilation
------------

This project is intended to be built using CMake the following way.

    cmake .
    make

Tests
-----

    cd test
    ../bin/app test.cpp -class-to-expand=C -add-default-value -add-remind-virtual -- -std=c++11

Help
----

    ../bin/app --help


