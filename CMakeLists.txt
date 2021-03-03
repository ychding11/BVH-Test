cmake_minimum_required (VERSION 3.8)
project (BVH-Lab)

set(CMAKE_CXX_STANDARD 17)

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON) 

set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo" CACHE STRING "Configuration types" FORCE)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")

option(ENABLE_HIDECONSOLE_BUILD "Enable to hide console on Windows" OFF)

file(GLOB SRC_FILES
    ${CMAKE_SOURCE_DIR}/src/*.h
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/visualize/*.h
    ${CMAKE_SOURCE_DIR}/visualize/*.cpp
)

file(GLOB EXT_FILES
    ${CMAKE_SOURCE_DIR}/thirdparty/*.h
    ${CMAKE_SOURCE_DIR}/thirdparty/lodepng/*.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/*.h
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/*.cpp
	${CMAKE_SOURCE_DIR}/thirdparty/glew/src/glew.c
    ${CMAKE_SOURCE_DIR}/thirdparty/fastnoise/*cpp
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/visualize
  ${CMAKE_SOURCE_DIR}/thirdparty/
  ${CMAKE_SOURCE_DIR}/thirdparty/lodepng
  ${CMAKE_SOURCE_DIR}/thirdparty/bvh/include
  ${CMAKE_SOURCE_DIR}/thirdparty/spdlog/include
  ${CMAKE_SOURCE_DIR}/thirdparty/glm
  ${CMAKE_SOURCE_DIR}/thirdparty/fonts
  ${CMAKE_SOURCE_DIR}/thirdparty/imgui
  ${CMAKE_SOURCE_DIR}/thirdparty/glfw/include
  ${CMAKE_SOURCE_DIR}/thirdparty/glew/include
  ${CMAKE_SOURCE_DIR}/thirdparty/fastnoise
)

set(GLFW_LIBS glfw3.lib)

## Here Should Set correct value based on Visual Studio Version
link_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glfw/lib-vc2015
)

find_package(OpenGL)

#######################################################################################

#set (USE_MSVC_RUNTIME_LIBRARY_DLL true)
#IF (NOT USE_MSVC_RUNTIME_LIBRARY_DLL)
#    #We statically link to reduce dependancies
#    FOREACH(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_RETAIL)
#        IF(${flag_var} MATCHES "/MD")
#            STRING(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
#        ENDIF(${flag_var} MATCHES "/MD")
#        IF(${flag_var} MATCHES "/MDd")
#            STRING(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
#        ENDIF(${flag_var} MATCHES "/MDd")
#    ENDFOREACH(flag_var)
#ENDIF (NOT USE_MSVC_RUNTIME_LIBRARY_DLL)

#######################################################################################

foreach(f ${SRC_FILES})
    # Get the path of the file relative to ${DIRECTORY},
    # then alter it (not compulsory)
    file(RELATIVE_PATH SRCGR ${CMAKE_SOURCE_DIR} ${f})
    set(SRCGR "Sources/${SRCGR}")

    # Extract the folder, ie remove the filename part
    string(REGEX REPLACE "(.*)(/[^/]*)$" "\\1" SRCGR ${SRCGR})

    # Source_group expects \\ (double antislash), not / (slash)
    string(REPLACE / \\ SRCGR ${SRCGR})
    source_group("${SRCGR}" FILES ${f})
endforeach()

if(WINDOWS)
  set(GUI_TYPE WIN32)
elseif(MACOSX)
  set(GUI_TYPE MACOSX_BUNDLE)
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()


set(LINK_OPTIONS " ")

set(EXE_NAME "BVH-Test")

ADD_EXECUTABLE(${EXE_NAME} ${SRC_FILES} ${EXT_FILES})

TARGET_LINK_LIBRARIES(${EXE_NAME} ${OPENGL_LIBRARIES} ${GLFW_LIBS})

#--------------------------------------------------------------------
# preproc : add macros
#--------------------------------------------------------------------
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-D__STDC_FORMAT_MACROS)
add_definitions(-DBX_CONFIG_ENABLE_MSVC_LEVEL4_WARNINGS=1)
add_definitions(-D__STDC_LIMIT_MACROS)
add_definitions(-D__STDC_CONSTANT_MACROS)
add_definitions(-DIMGUI_DISABLE_OBSOLETE_FUNCTIONS)
if(MSVC)
    add_definitions(-DWIN32)
    add_definitions(-D_WIN32)
    #add_definitions(-DNOMINMAX)
endif()
add_definitions(-DUSE_DL_PREFIX)
add_definitions(-DGLEW_STATIC)

#--------------------------------------------------------------------
# target properties : output dirs
#--------------------------------------------------------------------
set_target_properties(${EXE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin )
set_target_properties(${EXE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin )
set_target_properties(${EXE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_SOURCE_DIR}/bin )
set_target_properties(${EXE_NAME} PROPERTIES DEBUG_POSTFIX "_d")
set_target_properties(${EXE_NAME} PROPERTIES RELWITHDEBINFO_POSTFIX "RelWithDebInfo")
set_target_properties(${EXE_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

#--------------------------------------------------------------------
# Hide the console window in visual studio projects
#--------------------------------------------------------------------
if(ENABLE_HIDECONSOLE_BUILD)
    if(MSVC)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
    endif()
endif()

if(ENABLE_HIDECONSOLE_BUILD)
    message(STATUS "Console is hidden")
else()
    message(STATUS "Console is visible")
endif()

#######################################################################################
##For reference
##https://cmake.org/cmake/help/latest/manual/cmake-properties.7.html#target-properties
#######################################################################################
