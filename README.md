# Introduction

This demo shows an algorithm to check whether a vector of strings contains a certain substring. Vector is implemented in inc/vector.hpp. An event handler waits for user input and searches the vector for strings which contain the input as substring. The vector is initialized as strings `{"AAAA", "BAAA", "CAAA", ... , "ZZZX, "ZZZY"; "ZZZZ"}`. 

# Compilation

Compile on windows (`windows.h` is used) and MinGW64 with:

```
g++ -I inc -o parallel-search src/*
```

or with visual studio and CMake.

# Execution

Run with
```
./parallel-search <n>
```

with `n` being the default number of threads used when searching for strings. An input data called `input.txt` will be generated and the output will be saved in separate `output<X>.txt` files with `X` being a file counter.

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)