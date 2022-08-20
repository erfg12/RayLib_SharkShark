Using nxdk to compile game for the original Xbox.

### How To Build
- Clone [nxdk](https://github.com/XboxDev/nxdk) contents into this directory.
- Windows use WSL Ubuntu and type `sudo apt install build-essential cmake flex bison clang lld git llvm`. MacOS: `brew install llvm cmake coreutils`
- Now type `./bin/activate` then `make` and it should compile.