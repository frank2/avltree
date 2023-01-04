# avltree

An AVL tree implementation for C++!

An **[AVL tree](https://en.wikipedia.org/wiki/AVL_tree)** is a self-balancing binary tree structure.
It can be the basis of many types of data structures. For example, a
**[red-black tree](https://en.wikipedia.org/wiki/Red%E2%80%93black_tree)**-- which is a similar data
structure to an AVL tree-- is the basis for many common C++ data structures, such as mappings and sets.

This library attempts to provide a basic, customizable AVL tree for the implementation of other data
structures which call for a self-balancing tree, such as an
[interval tree](https://en.wikipedia.org/wiki/Interval_tree).

## Requirements

The only requirement is the C++17 standard. Other than that it should be fine.

## Documentation

There are two ways to view the documentation. The most recent version of the documentation is hosted on
[my github.io page](https://frank2.github.io/docs/avltree). Alternatively, you can revert to a specific
version tag within the repository and view the documentation stored in the `doc` folder. Currently, the
only version is 1.0, but this may change in the future if bugs are found or if additional features wish
to be added.

## Building

This library makes use of [CMake](https://cmake.org) for easy integration into other projects. If you'd
rather not use CMake, however, the library is written in such a way that it is simply one header and can
be dropped anywhere you like.

Integrating this library into your CMake project is easy. First, add the subdirectory of the library:
```
add_subdirectory(path/to/avltree)
```

Then, add the link target to your link libraries:
```
target_link_libraries(your_project INTERFACE libavltree)
```

Simply `#include <avltree.hpp>` in your project after that and you should be able to build your project
with CMake.

## Testing

This library has been tested with Visual Studio 2019 and gcc-g++ 8.3.0
If you find yourself wishing to contribute to the library, you can run a test build with CMake:

```
$ mkdir build
$ cd build
$ cmake ../ -DTEST_AVLTREE=ON
$ cmake --build ./
$ ctest ./
```

The last line can be replaced with `ctest -C Debug ./` when building with Visual Studio.
