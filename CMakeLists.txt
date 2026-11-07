cmake_minimum_required(VERSION 3.22)
project(AcidLine303 VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use a local JUCE checkout if present, else an installed JUCE package, else FetchContent.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
  add_subdirectory(JUCE)
else()
  find_package(JUCE CONFIG QUIET)
  if(NOT JUCE_FOUND)
    include(FetchContent)
    FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.15 GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
  endif()
endif()

juce_add_plugin(AcidLine303
  COMPANY_NAME "Independent Audio"
  BUNDLE_ID "audio.independent.acidline303"
  PLUGIN_MANUFACTURER_CODE IndA
  PLUGIN_CODE AcD3
  FORMATS VST3 Standalone
  PRODUCT_NAME "AcidLine 303"
  IS_SYNTH TRUE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
  COPY_PLUGIN_AFTER_BUILD FALSE)

juce_generate_juce_header(AcidLine303)
target_sources(AcidLine303 PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_compile_definitions(AcidLine303 PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(AcidLine303 PRIVATE juce::juce_audio_utils juce::juce_dsp PUBLIC
  juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)
