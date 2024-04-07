cmake_minimum_required(VERSION 3.29)

#set vcpkg location
set(CMAKE_TOOLCHAIN_FILE "D:/MyWorspace/vcpkg/scripts/buildsystems/vcpkg.cmake")
#set(CMAKE_LIBRARY_PATH "D:/MyWorspace/vcpkg/installed/x64-windows/lib")
#create project
project(gaussian_splatting LANGUAGES CXX)

# varaibles
set(SOURCE main.cpp)
set(NAME_EXE Gaussian-Splatting)
#set(LIBRARY1 fmt)
#set(LIBRARY2 directxtk12)
set(VCPKG_JSON vcpkg.json)

#find_package(${LIBRARY1} REQUIRED)
#find_package(${LIBRARY2} REQUIRED)
macro(dynamic_dependencies __PATH__)
	# Lese die vcpkg.json-Datei ein
	file(READ "${CMAKE_CURRENT_SOURCE_DIR}/${__PATH__}" VCPKG_JSON_CONTENTS)
	
	# Finde index starts with "dependencies": [ 
	string(FIND "${VCPKG_JSON_CONTENTS}" "\"dependencies\": [" DEPENDENCIES_START_POS)
	math(EXPR DEPENDENCIES_START_POS "${DEPENDENCIES_START_POS} + 17")
	string(SUBSTRING "${VCPKG_JSON_CONTENTS}" ${DEPENDENCIES_START_POS} -1 DEPENDENCIES_SUBSTRING)
	# Finde index starts with ]
	string(FIND "${DEPENDENCIES_SUBSTRING}" "]" DEPENDENCIES_END_POS)
	string(SUBSTRING "${DEPENDENCIES_SUBSTRING}" 0 ${DEPENDENCIES_END_POS} EXTRACTED_DEPENDENCIES)
	
	# Entferne Anführungszeichen und Leerzeichen
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
endmacro()

dynamic_dependencies(${VCPKG_JSON})
#add_executable(${NAME_EXE} ${SOURCE})
#target_link_libraries(${NAME_EXE} PRIVATE ${LIBRARY1}::${LIBRARY1})# ${LIBRARY2}::${LIBRARY2})
#target_link_libraries(${NAME_EXE} PRIVATE Microsoft::DirectXTK12)
