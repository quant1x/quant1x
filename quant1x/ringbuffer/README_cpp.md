Vyukov MPMC queue — C++ demo

This folder contains a header-only port of the Vyukov bounded MPMC queue as
`vyukov.hpp` and a simple demo program at `examples/vyukov_demo.cpp`.

Building the demo (MSVC, PowerShell):

cl /std:c++17 /O2 /EHsc examples\vyukov_demo.cpp /Fe:vyukov_demo.exe

Building the demo (g++/clang++):

g++ -std=c++17 -O3 examples/vyukov_demo.cpp -o vyukov_demo -pthread

Run:

./vyukov_demo

Notes:
- The implementation is a direct translation of the Rust Vyukov MPMC queue
  using std::atomic and aligned storage. It provides try_push/try_pop
  non-blocking semantics and a close() method.
- This is header-only and intended for inclusion into existing C++ code.
