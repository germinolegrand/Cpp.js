Cpp.js is a project with no pretention. It aims to be a JS interpreter in a C++ environment. The JS environment should be highly and easily manipulable from simple modern C++.

Currently it is still in an early stage. The Parser is advanced enough to support a basic Interpreter.

You will find examples in `src/tests.cpp`.

# Build

For Code::Blocks (see https://github.com/chris-be/premake-codeblocks):
```
# cd build
# premake5 codeblocks
```

For Linux:
```
# cd build
# premake5-linux64 gmake2
# make
```

For MinGW64:
```
# cd build
# premake5 gmake2
# mingw32-make
```
