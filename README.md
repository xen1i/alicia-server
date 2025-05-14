# Alicia Server
Server, yippie!

## Community
If you're a programmer [join the dedicated developer](https://discord.gg/bP4pNWvVHM) community on Discord.

## Requirements

- Git
- CMake
- A C++20 compatible compiler
- Boost
- libpq

## Building

1. Clone the repository
```bash
git clone https://github.com/Story-Of-Alicia/alicia-server.git
```
2. Initialize the repository's submodules 
```bash
git submodule update --init --recursive
```
3. Create the build directory 
```bash
mkdir build; cd build
```
4. Generate the build files
```bash
cmake ..
```
5. Build the project
```bash
cmake --build .
```

After building, the executable `alicia-server` or `alicia-server.exe` will be present in the `build/` directory
