# ToDo
```diff
- Please clone the repo and follow this README.
```
1. install CMake
2. Install vcpkg

#add new libraries


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
The current CMake version (3.29.0) should now be displayed on the console.

In the file "CMakeList.txt" is stored that older CMake versions than v3.29.0 are not activated. 

# vcpkg
## Install vcpkg
1. clone the repo from Github (for example next to the repo "GaussianSplatting")
```
git clone https://github.com/microsoft/vcpkg.git
```
2. navigate to the repo and execute the bootstrap-vcpkg.bat file
```
bootstrap-vcpkg.bat
```
3. test if vcpkg works
```
./vcpkg version
```
Create a system variable with the name `VCPKG_PATH` which is located on the vcpkg.cmake file in the currently installed repository.
For example: `D:/MyWorspace/vcpkg/scripts/buildsystems/vcpkg.cmake`.

## use vcpkg
there is a `vcpkg.json` file in the repo where the necessary dependencies can be added to the list. In addition, these must be installed in the vcpkg repo as described on the website.

Dependencies can be searched for here: https://vcpkg.io/en/packages

1. navigate to the folder/repo "vcpkg" on the console
2. install all necessary packages for the project on your computer: 			
	!Make sure that you have to adjust your path!
```
.\vcpkg install @D:/MyWorspace/GaussianSplatting/requirements.txt
```

If all packages are successfully installed on your computer, the CMakeLists.txt file should find them and include them in the project when creating/compiling.

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

