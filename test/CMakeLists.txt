cmake_minimum_required(VERSION 3.22)

# REMOVED: A subdirectory should NOT have its own project() command.
# This was redefining ${PROJECT_NAME} and breaking the link to the root project.

# Signals to CMake that we want to run tests from this directory.
enable_testing()

# Creates the test console application. We use the explicit name for clarity.
set(SOURCE_FILES source/AudioProcessorTest.cpp)
add_executable(AudioPluginTest ${SOURCE_FILES})

# --- Dependency Linking (The Main Fix) ---

# Sets the necessary include directories for googletest itself. This is correct.
target_include_directories(AudioPluginTest PRIVATE ${GOOGLETEST_SOURCE_DIR}/googletest/include)

# CORRECTED: This is the most important fix.
# We link our test executable against our main plugin target ('dynamics-doctor').
# This makes the test target automatically inherit all of the PUBLIC dependencies
# from the plugin, including JUCE, ebur128, include paths, and compile definitions.
target_link_libraries(AudioPluginTest PRIVATE
    dynamics-doctor       # <<< The crucial link to your plugin
    GTest::gtest_main     # Links against googletest
)

# REMOVED: These lines are now redundant. The correct include paths are
# inherited automatically from the 'dynamics-doctor' target we linked against above.
# target_include_directories(AudioPluginTest PRIVATE
#     ${CMAKE_SOURCE_DIR}/libs/juce/modules
#     ${CMAKE_SOURCE_DIR}/plugin/include
# )

# REMOVED: This is also redundant. The test executable will inherit the necessary
# warning flags from the dynamics-doctor target.
# set_source_files_properties(${SOURCE_FILES} PROPERTIES COMPILE_OPTIONS "${PROJECT_WARNINGS_CXX}")

# --- Google Test Discovery ---

# Adds googletest-specific CMake commands.
include(GoogleTest)

# Add all tests to CMake metadata so they can be run with ctest.
# CORRECTED: Use the explicit target name 'AudioPluginTest'.
if(CMAKE_GENERATOR STREQUAL Xcode)
  gtest_discover_tests(AudioPluginTest DISCOVERY_MODE PRE_TEST)
else()
  gtest_discover_tests(AudioPluginTest)
endif()