# Invoice Management

## Build

### Dependencies

- CMake >= 3.14
- SQLite3
- PCRE2

### Windows

using Powershell and CMake

~~~Powershell
$env:SQLite3_ROOT = "<Path to SQLite3 Install Directory>"
$env:PCRE2_ROOT   = "<Path to PCRE2 Install Directory>"
mkdir build
cd build
cmake ..
cmake --build .
cmake --install .
~~~

### Unix

~~~bash
mkdir build
cd build
cmake ..
cmake --build .
cmake --install .
~~~

## License

Licesned under [MIT](/LICENSE)
