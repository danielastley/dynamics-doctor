cmake_minimum_required(VERSION 3.22)

# Sets a few variables, like PROJECT_NAME
project(dynamics-doctor VERSION 0.1.0)

# CORRECTED: Set to C++20 for guaranteed cross-platform compatibility with JUCE 8.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# I like to download the dependencies to the same folder as the project.
set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs)

# Downloads CPM if not already downloaded.
include(cmake/cpm.cmake)

# This commands downloads AND configures JUCE.
cpmaddpackage(
  NAME
  JUCE
  GIT_TAG
  8.0.6 # Use the tag for JUCE 8
  VERSION
  8.0.6
  GITHUB_REPOSITORY
  juce-framework/JUCE
  SOURCE_DIR
  ${LIB_DIR}/juce
)

# Adds googletest.
cpmaddpackage(
  NAME
  GOOGLETEST
  GITHUB_REPOSITORY
  google/googletest
  VERSION
  1.16.0
  SOURCE_DIR
  ${LIB_DIR}/googletest
  OPTIONS
  "INSTALL_GTEST OFF"
  "gtest_force_shared_crt ON"
)

# REMOVED: Redundant and overly broad. We handle compiler options on a per-target
# basis inside the subdirectories, which is the correct modern CMake practice.
# include(cmake/CompilerWarnings.cmake)
# include(cmake/Util.cmake)
# add_compile_options(-Wno-error=old-style-cast)

# Adds all the targets configured in the "plugin" folder.
# The fixes for compiler warnings are correctly located inside this file.
add_subdirectory(plugin)

# This command allows running tests from the "build" folder.
enable_testing()

# Adds all the targets configured in the "test" folder.
add_subdirectory(test)

# REMOVED: This entire block was incorrect. The targets `${PROJECT_NAME}` and `ebur128`
# are not visible in this file's scope. This code would have produced a CMake error.
# The necessary `target_compile_options` are already correctly placed in `plugin/CMakeLists.txt`.