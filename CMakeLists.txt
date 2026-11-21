cmake_minimum_required(VERSION 3.22)
project(MaskBass VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
    GIT_SHALLOW ON)
FetchContent_MakeAvailable(JUCE)

# VST3 siempre. AAX (Pro Tools) solo si le pasas la ruta del AAX SDK de Avid:
#   cmake -B build -DAAX_SDK_PATH="C:/SDKs/AAX_SDK"
set(MB_FORMATS VST3)
set(AAX_SDK_PATH "" CACHE PATH "Ruta al Avid AAX SDK (opcional)")
if(AAX_SDK_PATH)
    juce_set_aax_sdk_path("${AAX_SDK_PATH}")
    list(APPEND MB_FORMATS AAX)
endif()

juce_add_plugin(MaskBass
    COMPANY_NAME "MaskAudio"
    PLUGIN_MANUFACTURER_CODE Mskb
    PLUGIN_CODE Mbas
    FORMATS ${MB_FORMATS}
    PRODUCT_NAME "Mask Bass"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    VST3_CATEGORIES Fx Dynamics
    AAX_CATEGORY AAX_ePlugInCategory_Dynamics
    COPY_PLUGIN_AFTER_BUILD FALSE)

target_sources(MaskBass PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(MaskBass PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

juce_generate_juce_header(MaskBass)

target_link_libraries(MaskBass
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags)
