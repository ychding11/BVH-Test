cmake_minimum_required (VERSION 3.6)
project (BVHTest)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo" CACHE STRING "Configuration types" FORCE)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON) 

## 3rd party moudle
#######################################################################################
if(NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glog")
  message(FATAL_ERROR "The glog submodule directory is missing! "
    "You probably did not clone the project with --recursive, or first checked out "
    "before it was added. It is possible to recover by running "
    "\"git submodule update --init --recursive\"")
endif()

if(WIN32)
  add_definitions( -D GOOGLE_GLOG_DLL_DECL= )
endif()
set(WITH_GFLAGS OFF CACHE BOOL "Use gflags")
set(BUILD_SHARED_LIBS OFF CACHE BOOL " " FORCE)
set(BUILD_TESTING OFF CACHE BOOL "Build the glog unit test.")
#message(STATUS "OPT BEFORE=${BUILD_TESTING}")
add_subdirectory(thirdparty/glog)
#message(STATUS "OPT AFTER=${BUILD_TESTING}")
set_property(TARGET glog PROPERTY FOLDER "dependencies")
include_directories (
  ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glog/src
  ${CMAKE_BINARY_DIR}/thirdparty/glog
)


file(GLOB  EXE_SRC_FILES
	${CMAKE_SOURCE_DIR}/raytest/*.cpp
	${CMAKE_SOURCE_DIR}/raytest/*.h
)
file(GLOB  HW_SRC_FILES
	${CMAKE_SOURCE_DIR}/quad/*.h
    ${CMAKE_SOURCE_DIR}/quad/*.cpp
)
file(GLOB SRC_FILES
    ${CMAKE_SOURCE_DIR}/src/*.h
    ${CMAKE_SOURCE_DIR}/src/*.cpp
	${CMAKE_SOURCE_DIR}/src/core/*.h
	${CMAKE_SOURCE_DIR}/src/core/*.cpp
    ${CMAKE_SOURCE_DIR}/src/random/*.h
	${CMAKE_SOURCE_DIR}/src/random/*.cpp
    ${CMAKE_SOURCE_DIR}/src/types/*.h
	${CMAKE_SOURCE_DIR}/src/types/*.cpp
	${CMAKE_SOURCE_DIR}/src/camera/*.h
	${CMAKE_SOURCE_DIR}/src/camera/*.cpp
	${CMAKE_SOURCE_DIR}/src/integrator/*.h
	${CMAKE_SOURCE_DIR}/src/integrator/*.cpp
	${CMAKE_SOURCE_DIR}/src/accelerator/*.h
	${CMAKE_SOURCE_DIR}/src/accelerator/*.cpp
	${CMAKE_SOURCE_DIR}/src/utils/*.h
	${CMAKE_SOURCE_DIR}/src/utils/*.cpp
)

file(GLOB EXT_FILES
    ${CMAKE_SOURCE_DIR}/thirdparty/*.h
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/*.h
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/*.cpp
	${CMAKE_SOURCE_DIR}/thirdparty/glew/src/glew.c
    ${CMAKE_SOURCE_DIR}/thirdparty/fastnoise/*cpp
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/
  ${CMAKE_SOURCE_DIR}/src/
  ${CMAKE_SOURCE_DIR}/src/core/
  ${CMAKE_SOURCE_DIR}/src/types/
  ${CMAKE_SOURCE_DIR}/quad/
  ${CMAKE_SOURCE_DIR}/thirdparty/
  ${CMAKE_SOURCE_DIR}/thirdparty/glm
  ${CMAKE_SOURCE_DIR}/thirdparty/imgui
  ${CMAKE_SOURCE_DIR}/thirdparty/glfw/include
  ${CMAKE_SOURCE_DIR}/thirdparty/glew/include
  ${CMAKE_SOURCE_DIR}/thirdparty/fastnoise
)

## Output include directory for debug
#######################################################################################
get_property(includes DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${includes})
    message(STATUS "include dir='${dir}'")
endforeach()

## macro definition
#######################################################################################
macro(group_src_files src_files target)
	foreach(f ${src_files})
		message(STATUS "check parameters original source file :${f}")
	endforeach()
	message(STATUS "check parameters new group name :${target}")
    
    foreach(f ${src_files})
		# Get the path of the file relative to ${DIRECTORY},
		# then alter it (not compulsory)
		file(RELATIVE_PATH SRCGR ${CMAKE_SOURCE_DIR} ${f})
		set(SRCGR "${target}/${SRCGR}")
		
		# Extract the folder, ie remove the filename part
		string(REGEX REPLACE "(.*)(/[^/]*)$" "\\1" SRCGR ${SRCGR})

		# Source_group expects \\ (double antislash), not / (slash)
		string(REPLACE / \\ SRCGR ${SRCGR})
		source_group("${SRCGR}" FILES ${f})
		message(STATUS "New group name :${SRCGR}")
	endforeach()
endmacro()

## Regroup source code structure
#######################################################################################
group_src_files("${SRC_FILES}" "Mei")
group_src_files("${EXT_FILES}"  "External")
group_src_files("${EXE_SRC_FILES}"  "Render")
group_src_files("${HW_SRC_FILES}" "HWRender")

## Here Should Set correct value based on Visual Studio Version
link_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glfw/lib-vc2015
)

find_package(OpenGL)

set(GLFW_LIBS glfw3.lib)
#######################################################################################


#######################################################################################

set(ENABLE_HIDECONSOLE_BUILD FALSE CACHE BOOL "TRUE to hide console for Windows sys" FORCE)
if(ENABLE_HIDECONSOLE_BUILD)
	message(STATUS "Console is hidden")
else()
	message(STATUS "Console is visible")
endif()

if(WINDOWS)
  set(GUI_TYPE WIN32)
elseif(MACOSX)
  set(GUI_TYPE MACOSX_BUNDLE)
endif()

find_package(OpenMP)
if(OPENMP_FOUND)
	if(MSVC)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /openmp")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp")
		#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
	endif()
endif()

if(MSVC)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

## Linker Options
#######################################################################################
if(MSVC)
    if(ENABLE_HIDECONSOLE_BUILD)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
    else()
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE ")
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE")
    endif()
endif(MSVC)
SET(LINK_OPTIONS " ")

## Macro Defines
#######################################################################################
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-D__STDC_FORMAT_MACROS)
add_definitions(-DBX_CONFIG_ENABLE_MSVC_LEVEL4_WARNINGS=1)
add_definitions(-D__STDC_LIMIT_MACROS)
add_definitions(-D__STDC_CONSTANT_MACROS)
add_definitions(-DIMGUI_DISABLE_OBSOLETE_FUNCTIONS)
add_definitions(-DUSE_DL_PREFIX)
add_definitions(-DGLEW_STATIC)
if(${ENABLE_HIDECONSOLE_BUILD})
	if(MSVC)
		add_definitions(-DWIN32)
		add_definitions(-D_WIN32)
		add_definitions(-DNOMINMAX)
	endif()
endif()


macro(default_property EXE_NAME)
    message(STATUS "set default output property of target:${EXE_NAME}")
    set_target_properties(${EXE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin )
    set_target_properties(${EXE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin )
    set_target_properties(${EXE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_SOURCE_DIR}/bin )
    set_target_properties(${EXE_NAME} PROPERTIES DEBUG_POSTFIX "_d")
    set_target_properties(${EXE_NAME} PROPERTIES RELWITHDEBINFO_POSTFIX "RelWithDebInfo")
    set_target_properties(${EXE_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
	if(ENABLE_HIDECONSOLE_BUILD)
		if(WINDOWS)
			set_target_properties(${EXE_NAME} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
			set_target_properties(${EXE_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
		endif()
	endif()
endmacro()

## Target
#######################################################################################
set(EXE_NAME "MeiRender")
add_executable(${EXE_NAME} ${EXE_SRC_FILES} ${HW_SRC_FILES} ${SRC_FILES} ${EXT_FILES})
target_link_libraries(${EXE_NAME} ${OPENGL_LIBRARIES} ${GLFW_LIBS} glog)
default_property(${EXE_NAME})

## Tests
#######################################################################################
file ( GLOB TEST_SOURCE
    ${CMAKE_SOURCE_DIR}/tests/*.cpp
    ${CMAKE_SOURCE_DIR}/tests/gtest/*.cc
  )
set(EXE_NAME "mytest")
add_executable ( ${EXE_NAME} ${TEST_SOURCE} ${SRC_FILES})
target_link_libraries( ${EXE_NAME} glog )
default_property(${EXE_NAME})


#######################################################################################
##For reference
##https://cmake.org/cmake/help/latest/manual/cmake-properties.7.html#target-properties
#######################################################################################
