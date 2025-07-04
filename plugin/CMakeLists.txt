cmake_minimum_required(VERSION 3.22)

# Version is needed by JUCE.
project(dynamics-doctor VERSION 0.1.0)

# Adding a directory with the library/application name as a subfolder of the
# include folder is a good practice. It helps avoid name clashes later on.
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/DynamicsDoctor")

# Adds a plugin target (that's basically what the Projucer does).
juce_add_plugin(
  ${PROJECT_NAME}
  COMPANY_NAME
  "DMA"
  IS_SYNTH
  FALSE
  NEEDS_MIDI_INPUT
  FALSE
  NEEDS_MIDI_OUTPUT
  FALSE
  PLUGIN_MANUFACTURER_CODE
  Dmap
  PLUGIN_CODE
  Dmad
  FORMATS
  Standalone
  VST3
  AU
  PRODUCT_NAME
  "Dynamics Doctor"
)

# This is the correct fix for your build error. It tells the compiler to
# ignore the specific "double-promotion" warning that was being treated as an error.
target_compile_options(${PROJECT_NAME} PRIVATE
    "-Wno-error=double-promotion"  # This fixes the build failure
    "-Wno-old-style-cast"          # (Optional) This cleans up the warning spam
)

target_sources(${PROJECT_NAME}
PRIVATE
  source/LoudnessMeter.cpp
  source/PluginProcessor.cpp
  source/PluginEditor.cpp
  source/TrafficLightComponent.cpp
  include/DynamicsDoctor/LoudnessMeter.h
  include/DynamicsDoctor/PluginProcessor.h
  include/DynamicsDoctor/PluginEditor.h
  include/DynamicsDoctor/TrafficLightComponent.h
  include/DynamicsDoctor/Constants.h
  ThirdParty/libebur128/ebur128/ebur128.h)

# Sets the include directories of the plugin project.
target_include_directories(${PROJECT_NAME}
PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# --- Third-party library ebur128 ---
add_library(ebur128
    ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/libebur128/ebur128/ebur128.c
)

# CORRECTED: Add the include directory to your main project, not a non-existent one.
target_include_directories(${PROJECT_NAME} PUBLIC
    # This path is for the main ebur128.h header
    ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/libebur128/ebur128

    # This NEW path is for our Windows compatibility header (sys/queue.h)
    ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/libebur128/compat
)

# Tell our main plugin target where to find the compatibility header
target_include_directories(dynamics-doctor PRIVATE 
    "ThirdParty/libebur128/compat"
)

# CORRECTED: This was a typo. The command is target_link_libraries.
# Links to all necessary dependencies.
target_link_libraries(${PROJECT_NAME}
PUBLIC
    juce::juce_audio_utils
)

target_link_libraries(
  ${PROJECT_NAME} PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags
                         juce::juce_recommended_warning_flags # This is where -Werror comes from
)

# CORRECTED: Link your main project to the ebur128 library.
target_link_libraries(${PROJECT_NAME} PUBLIC ebur128)

# These definitions are recommended by JUCE.
target_compile_definitions(${PROJECT_NAME} PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)

# REMOVED: This line was broken because ${SOURCE_FILES} was never defined.
# The warning flags are correctly handled by juce::juce_recommended_warning_flags above.

# In Visual Studio this command provides a nice grouping of source files in "filters".
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/..)
