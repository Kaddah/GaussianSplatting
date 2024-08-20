cmake_minimum_required(VERSION 3.29)

set(PKGS directxtk12 glm imgui)

set(VCPKG_INSTALL_PATH "vcpkg")
#D:/MyWorspace/vcpkg/scripts/buildsystems/vcpkg.cmake
# Finde die Position von "vcpkg" im String


if (DEFINED ENV{VCPKG_PATH})
	# Finde die Position von "vcpkg" im String
	string(FIND "$ENV{VCPKG_PATH}" "vcpkg" INDEX)
	if (${INDEX} GREATER -1)
		math(EXPR substring_length "${INDEX} + 5")
		string(SUBSTRING $ENV{VCPKG_PATH} 0 ${substring_length} SUBSTRING_PATH)
	endif ()
	string(REPLACE "\\" "/" SUBSTRING_PATH_ "${SUBSTRING_PATH}")
	set(VCPKG_INSTALL_PATH ${SUBSTRING_PATH_})
	message(STATUS "Found path for vcpg repository:  ${SUBSTRING_PATH_}")
endif()

include(FetchContent)
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
  cmake_policy(SET CMP0135 NEW)
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_UNICODE /DUNICODE")

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)

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
	VCPKG_DIR ${VCPKG_INSTALL_PATH}
)
#create project
project(gaussian_splatting LANGUAGES CXX)

file(GLOB_RECURSE LOAD_FILES shader/*.hlsl assets/*.ply)
file(GLOB SOURCE header/*.h src/*.cpp)
set(NAME_EXE Gaussian-Splatting)


foreach(PKG IN ITEMS ${PKGS})
	find_package(${PKG} REQUIRED)
endforeach(PKG)

# config Debugger


# generate .exe
add_executable(${NAME_EXE} ${SOURCE})

#load files to project (sln)
file(GLOB_RECURSE SHADER_FILES "shader/*.hlsl")
source_group("Source Files/shader" FILES ${SHADER_FILES})
file(GLOB_RECURSE ASSETS_FILES "assets/*.ply")
source_group("Source Files/assets" FILES ${ASSETS_FILES})
target_sources(${NAME_EXE} PRIVATE ${SHADER_FILES} ${ASSETS_FILES})

# Exclude from build
file(GLOB_RECURSE EXCLUDE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/shader/*" "${CMAKE_CURRENT_SOURCE_DIR}/assets/*")
foreach(FILE ${EXCLUDE_FILES})
    set_source_files_properties(${FILE} PROPERTIES HEADER_FILE_ONLY TRUE)
endforeach()

target_link_libraries(${NAME_EXE} PRIVATE
    Microsoft::DirectXTK12
    glm::glm
    imgui::imgui)
	
# Specify that the subcompile should use WinMain
set_target_properties(${NAME_EXE} PROPERTIES WIN32_EXECUTABLE TRUE)

#make header avialable
target_include_directories(${NAME_EXE} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/header)

# Add files to build folder for executable
file(GLOB LOAD_FILES "shader/*.hlsl" "assets/*.ply" "compute/*")
foreach(_file IN ITEMS ${LOAD_FILES})
    get_filename_component(filename ${_file} NAME)
    get_filename_component(directory ${_file} DIRECTORY)
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" relative_directory ${directory})
    configure_file(${_file} ${CMAKE_CURRENT_BINARY_DIR}/${relative_directory}/${filename} COPYONLY)
endforeach(_file)

# Set CMAKE option
#option(DEFAULT_PLY_FILE "Default PLY file path" "../assets/cat.ply")

set(DEFAULT_PLY_FILE "../assets/cat.ply" CACHE STRING "Default PLY file path")


message(STATUS "Default PLY file: ${DEFAULT_PLY_FILE}")

# Set the CMake option as target argument for the debugger
set_target_properties(${NAME_EXE} PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "${DEFAULT_PLY_FILE}")

