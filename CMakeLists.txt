cmake_minimum_required(VERSION 3.29)

set(PKGS directxtk12 glm imgui)


include(FetchContent)
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
  cmake_policy(SET CMP0135 NEW)
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_UNICODE /DUNICODE")
# Add project_options from https://github.com/aminya/project_options
# Change the version in the following URL to update the package (watch the releases of the repository for future updates)
set(PROJECT_OPTIONS_VERSION "v0.35.1")
FetchContent_Declare(
  _project_options
  URL https://github.com/aminya/project_options/archive/refs/tags/${PROJECT_OPTIONS_VERSION}.zip)
FetchContent_MakeAvailable(_project_options)
include(${_project_options_SOURCE_DIR}/Index.cmake)

# install vcpkg dependencies: - should be called before defining project()
run_vcpkg(
    VCPKG_URL "https://github.com/microsoft/vcpkg.git"
    VCPKG_REV "10e052511428d6b0c7fcc63a139e8024bb146032"
)
#create project
project(gaussian_splatting LANGUAGES CXX)

set(SOURCE main.cpp DxException.cpp)

set(NAME_EXE Gaussian-Splatting)


foreach(PKG IN ITEMS ${PKGS})
	find_package(${PKG} REQUIRED)
endforeach(PKG)

# generate .exe
add_executable(${NAME_EXE} ${SOURCE})

target_link_libraries(${NAME_EXE} PRIVATE
    Microsoft::DirectXTK12
    glm::glm
    imgui::imgui)
	
# Specify that the subcompile should use WinMain
set_target_properties(${NAME_EXE} PROPERTIES WIN32_EXECUTABLE TRUE)

#load additional files
target_include_directories(${NAME_EXE} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${NAME_EXE} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/header)

set(HLSL_FILES VertexShader.hlsl PixelShader.hlsl)
# Add the HLSL files to your project
foreach(shader_file IN ITEMS ${HLSL_FILES})
	# Copy the HLSL files to the output directory for execution
	configure_file(${shader_file} ${CMAKE_CURRENT_BINARY_DIR}/Debug/${shader_file} COPYONLY)
	# Copy the shader files to the build directory
	configure_file(${shader_file} ${CMAKE_CURRENT_BINARY_DIR}/${shader_file} COPYONLY)
endforeach(shader_file)