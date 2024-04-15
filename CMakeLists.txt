cmake_minimum_required(VERSION 3.29)

#set vcpkg location
set(CMAKE_TOOLCHAIN_FILE "D:/MyWorspace/vcpkg/scripts/buildsystems/vcpkg.cmake")
#create project
project(gaussian_splatting LANGUAGES CXX)
#set unicode compiler
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_UNICODE /DUNICODE")
# set HLSL-compiler
set(HLSL_FLAGS "-T ps_4_0_level_9_1" CACHE STRING "HLSL Compiler Flags")

set_source_files_properties(PixelShader.hlsl PROPERTIES VS_SHADER_FLAGS ${HLSL_FLAGS})

# varaibles
set(HLSL_FILES VertexShader.hlsl PixelShader.hlsl)

set(SOURCE main.cpp)

set(NAME_EXE Gaussian-Splatting)

set(VCPKG_JSON vcpkg.json)


foreach(PACKAGE ${EXTRACTED_DEPENDENCIES})
		find_package(${PACKAGE} REQUIRED)
		message("load package: ${PACKAGE}")
	endforeach()
macro(dynamic_dependencies __PATH__)
	# read vcpkg.json-file
	file(READ "${CMAKE_CURRENT_SOURCE_DIR}/${__PATH__}" VCPKG_JSON_CONTENTS)
	
	# Find index starts with "dependencies": [ 
	string(FIND "${VCPKG_JSON_CONTENTS}" "\"dependencies\": [" DEPENDENCIES_START_POS)
	math(EXPR DEPENDENCIES_START_POS "${DEPENDENCIES_START_POS} + 17")
	string(SUBSTRING "${VCPKG_JSON_CONTENTS}" ${DEPENDENCIES_START_POS} -1 DEPENDENCIES_SUBSTRING)
	# Finde index starts with ]
	string(FIND "${DEPENDENCIES_SUBSTRING}" "]" DEPENDENCIES_END_POS)
	string(SUBSTRING "${DEPENDENCIES_SUBSTRING}" 0 ${DEPENDENCIES_END_POS} EXTRACTED_DEPENDENCIES)
	
	# formating
	string(REPLACE "\"" "" EXTRACTED_DEPENDENCIES "${EXTRACTED_DEPENDENCIES}")
	string(REPLACE " " "" EXTRACTED_DEPENDENCIES "${EXTRACTED_DEPENDENCIES}")
	string(REPLACE "\n" "" EXTRACTED_DEPENDENCIES "${EXTRACTED_DEPENDENCIES}")
	string(REPLACE "," ";" EXTRACTED_DEPENDENCIES "${EXTRACTED_DEPENDENCIES}")
	
	# Lade jedes Paket mit vcpkg
	foreach(PACKAGE ${EXTRACTED_DEPENDENCIES})
		find_package(${PACKAGE} REQUIRED)
		message("load package: ${PACKAGE}")
	endforeach()
	add_executable(${NAME_EXE} ${SOURCE})
	#lade zusatz files
	target_include_directories(${NAME_EXE} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
	target_include_directories(${NAME_EXE} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/header)
	#target_link_libraries(${NAME_EXE} PRIVATE user32 gdi32 kernel32)
	# Angeben, dass das Unterkompilat WinMain verwenden soll
	set_target_properties(${NAME_EXE} PROPERTIES WIN32_EXECUTABLE TRUE)
	#foreach(PACKAGE ${EXTRACTED_DEPENDENCIES})
	#	target_link_libraries(${NAME_EXE} PRIVATE ${PACKAGE}::${PACKAGE})
	#	message("load: ${PACKAGE}")
	#endforeach()
	foreach(DEPENDENCY ${EXTRACTED_DEPENDENCIES})
        message("link package: ${DEPENDENCY}")
		if (DEPENDENCY STREQUAL "directxtk12")
			target_link_libraries(${NAME_EXE} PRIVATE Microsoft::DirectXTK12)
		else()
			target_link_libraries(${NAME_EXE} PRIVATE ${DEPENDENCY}::${DEPENDENCY})
		endif()
    endforeach()
	
	# Fügen Sie die HLSL-Dateien Ihrem Projekt hinzu


	# Kopieren Sie die HLSL-Dateien in das Ausgabeverzeichnis für die Ausführung
	# Kopiere die Shader-Dateien in das Build-Verzeichnis
	foreach(shader_file IN ITEMS ${HLSL_FILES})
		configure_file(${shader_file} ${CMAKE_CURRENT_BINARY_DIR}/Debug/${shader_file} COPYONLY)
		configure_file(${shader_file} ${CMAKE_CURRENT_BINARY_DIR}/${shader_file} COPYONLY)
	endforeach(shader_file)
endmacro()

dynamic_dependencies(${VCPKG_JSON})
#add_executable(${NAME_EXE} ${SOURCE})
#target_link_libraries(${NAME_EXE} PRIVATE ${LIBRARY1}::${LIBRARY1})# ${LIBRARY2}::${LIBRARY2})
#target_link_libraries(${NAME_EXE} PRIVATE Microsoft::DirectXTK12)