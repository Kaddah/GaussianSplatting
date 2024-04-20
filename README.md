# ToDo
```diff
- Please clone the repo and follow this README.
```
1. install CMake

# CMake
## Install CMake
1. open cmd (as admin)
```
pip install cmake
```
2. check if everything works
```
cmake --version
```

# vcpkg

If you want to use new imports in the C++ script, these must be included in the `vcpkg.jon` and in the `CMakeLists.txt`.

In CMakeLists.txt, the new package must be inserted once in line 3: 
```
set(PKGS directxtk12 glm imgui)
```
and line 40 must also be adapted:
```
target_link_libraries(${NAME_EXE} PRIVATE
    Microsoft::DirectXTK12
    glm::glm
    imgui::imgui)
```

Dependencies can be searched for here: https://vcpkg.io/en/packages


# Create Visual Studio project

1. open cmd
2. navigate to the project (for example: D:\MyWorspace\GaussianSplatting)
```
cd D:\MyWorspace\GaussianSplatting
```
3. create a new folder in which the project is created (e.g. build)
```
mkdir build
```
5. navigate to the new folder
```
cd build
```
6. create the project here based on the files in the parent folder (CMakeLists.txt and main.cpp must be in the parent folder)
```
cmake ..
```

# Compile project

1. navigate to the project
```
cd D:\MyWorspace\GaussianSplatting
```
2. create an executable file
```
cmake --build build
```		
The .exe file can now be found in GaussianSplatting/build/Debug/Gaussian-Splatting.exe

