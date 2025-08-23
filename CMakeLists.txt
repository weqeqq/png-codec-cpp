
cmake_minimum_required(VERSION 3.10)
project(PngCodecCpp)

include(FetchContent)
include(GenerateExportHeader)

cmake_policy(SET CMP0169 OLD)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
FetchContent_Declare(
    wuffs
    GIT_REPOSITORY https://github.com/google/wuffs.git
    GIT_TAG        1e2e58cea012ea4c7553f327b63fafe196b0f9e2
)
FetchContent_Declare(
    fpng
    GIT_REPOSITORY https://github.com/richgel999/fpng.git
    GIT_TAG        925796543b9d26b8edfcdcecd94c1dac280f29fc
)
FetchContent_GetProperties(wuffs)
FetchContent_GetProperties(fpng)

function(Patch project_name)
    find_package(Git REQUIRED)
    set(patch_file "${CMAKE_CURRENT_SOURCE_DIR}/${project_name}.patch")
    set(patch_stamp "${CMAKE_CURRENT_BINARY_DIR}/${project_name}_patch_applied.stamp")

    if(NOT EXISTS "${patch_file}")
        message(FATAL_ERROR "Patch file ${patch_file} does not exist.")
    endif()
    if(EXISTS "${patch_stamp}")
        message(STATUS "Patch for ${project_name} already applied (stamp found)")
        return()
    endif()
    execute_process(
        COMMAND ${GIT_EXECUTABLE} -C ${${project_name}_SOURCE_DIR} apply ${patch_file}
        RESULT_VARIABLE error
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error_output
    )
    if(error)
        message(FATAL_ERROR "Failed to apply patch to ${project_name}:\n${error_output}")
    else()
        message(STATUS "Successfully applied patch to ${project_name}")
        file(WRITE "${patch_stamp}" "Patch applied on ${CMAKE_CURRENT_TIME}")
    endif()
endfunction()
foreach(library wuffs fpng)
    if(NOT ${${library}_POPULATED})
        FetchContent_Populate(${library})
        Patch(${library})
        add_subdirectory(${${library}_SOURCE_DIR} ${${library}_BINARY_DIR})
    endif()
endforeach()

add_library(PngCodec       OBJECT)
add_library(PngCodecShared SHARED)
add_library(PngCodecStatic STATIC)

target_compile_definitions(
    PngCodecShared
    PUBLIC
        PNG_CODEC_SHARED
    PRIVATE
        PNG_CODEC_STATIC
)
target_link_libraries(
    PngCodec
    PUBLIC
    PngCodecDecode
    PngCodecEncode
)
target_compile_features(PngCodec PUBLIC cxx_std_17)
target_sources(PngCodec
    PUBLIC
    FILE_SET HEADERS BASE_DIRS headers
    FILES
    headers/png-codec-cpp/png.h
    PRIVATE
    sources/png.cc
)
foreach(target PngCodecShared PngCodecStatic)
    target_link_libraries(
        ${target}
        PUBLIC
            PngCodec
            PngCodecDecode
            PngCodecEncode
    )
endforeach()
