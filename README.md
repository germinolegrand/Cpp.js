[![Build Status](https://travis-ci.org/germinolegrand/Cpp.js.svg?branch=master)](https://travis-ci.org/germinolegrand/Cpp.js)

Cpp.js is a project with no pretention. It aims to be a JS interpreter in a C++ environment. The JS environment should be highly and easily manipulable from simple modern C++.

```cpp
var{{
    {"console", {{
        {"log", var(
            [](auto args){
                std::copy(std::begin(args), std::end(args), std::ostream_iterator<var>(std::cout));
                std::cout << '\n';
                return var{};
            })
        }
    }}},
    {"exit", var(
        [](auto args)->var{
            exit(args.size() >= 1 ? static_cast<int>(args[0].to_double()) : 0);
        })
    }
}}
```

Currently it is still in an early stage, but is advanced enough to support a basic [Interpreter](console/main.cpp).

You will find examples in [`tests/`](tests/).

# Build

This repository has submodules, don't forget init them :
```
# git submodule update --init
```

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
