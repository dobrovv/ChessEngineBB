# Almost all CMake files should start with this
# You should always specify a range with the newest
# and oldest tested versions of CMake. This will ensure
# you pick up the best policies.
cmake_minimum_required(VERSION 3.1...3.25)

set (CMAKE_SYSTEM_VERSION 10.0.19041.0) # vsstudio

if (CMAKE_BUILD_TYPE STREQUAL "")
    # Build type is not set eg. command was "cmake .."
    message(STATUS "  Diag: Build type was unspecified, set to Release")
    set(CMAKE_BUILD_TYPE Release)
else ()
    message(STATUS "  Diag: Build type specified as '${CMAKE_BUILD_TYPE}'")
endif ()

if (${CMAKE_BUILD_TYPE} STREQUAL Debug)
    set_directory_properties(PROPERTIES COMPILE_DEFINITIONS "_DEBUG")
else ()
    set_directory_properties(PROPERTIES COMPILE_DEFINITIONS "NDEBUG")
endif ()

# This is your project statement. You should always list languages;
# Listing the version is nice here since it sets lots of useful variables
project(
  ChessEngineBB
  VERSION 0.2
  LANGUAGES CXX)

# LLVM SUPPORT  
#find_package(LLVM REQUIRED CONFIG)
#message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
#message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
#include_directories(${LLVM_INCLUDE_DIRS})
#add_definitions(${LLVM_DEFINITIONS})
message(STATUS "My Windows SDK version: ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}") # vsstudio
# If you set any CMAKE_ variables, that can go here.
# (But usually don't do this, except maybe for C++ standard)

# Find packages go here.

# You should usually split this into folders, but this is a simple example

# This is a "default" library, and will match the *** variable setting.
# Other common choices are STATIC, SHARED, and MODULE
# Including header files here helps IDEs but is not required.
# Output libname matches target name, with the usual extensions on your system
# add_library(MyLibExample simple_lib.cpp simple_lib.hpp)

# Link each target with other targets or add options, etc.

# Adding something we can run - Output name matches target name
add_executable(MyChessEngine main.cpp types.h bitboard.h position.h position.cpp board.h board.cpp movegen.h movegen.cpp "tests.cpp" "evaluate.h" "search.h" "zobrist.h" "zobrist.cpp" "tt.h" "search.cpp")

# Make sure you link your targets with this command. It can also link libraries and
# even flags, so linking a target that does not exist will not give a configure-time error.
# target_link_libraries(MyExample PRIVATE MyLibExample)


if (MSVC)
    # warning level 4 and all warnings as errors
    #add_compile_options(/W4 /WX /O2)

    SET(CMAKE_CXX_FLAGS_RELEASE "/Oy /O2")
    SET(CMAKE_CXX_FLAGS_DEBUG "/O2 /Zi")

    # enable CPU profiling
    #set_target_properties(${BUILD_TARGET} PROPERTIES LINK_FLAGS "/PROFILE")

else()
    # lots of warnings and all warnings as errors
    #add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

