header-expander
===============

Provide a simple header-expander for C++ classes written with libclang

For compilation 
cmake .
make

Test :
cd test
../bin/app test.cpp -class-to-expand=C -add-default-value -add-remind-virtual -- -std=c++11

Help :
../bin/app --help


