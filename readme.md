## About
The Shark! Shark! game rebuilt in C with RayLib.

## How To Build
### Windows
1. Install CMake, ensure the cmake command is now avaiable in command prompt.
2. Run `Windows_make_vs_proj.bat` this will generate a `build` directory with Visual Studio project files in it.
3. Open the `SharkShark.sln` file in Visual Studio 2019 or above. Click Build if you would like to build the application.

### MacOS & Linux
1. Install CMake, open CMake, click `Tools > How To Install For Command Line Use`. Follow one of those options.
2. Run `MacLinux_build.sh` this will generate a `build` directory and `Makefile` and automatically build the application.
3. For future building, open a Terminal in the `build` directory and use the `make` command to build the application.